#pragma once
#include <set>
#include <cctype>
#include <shlobj.h>
#include <windows.h>
#include <shobjidl_core.h>

#include "BaseScene.h"
#include "RipeGrain/UILayer.h"


class FireFLyeLoadedScene;

std::string convert_to_string(const std::wstring& wstr)
{
	if (wstr.empty())
		return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
std::wstring convert_to_wstring(const std::string& str)
{
	if (str.empty())
		return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
std::optional<std::string> ShowSelectFolderOrFileDialogue()
{ 
	BROWSEINFO bi = { 0 }; 
	bi.lpszTitle = "Select a File or Directory"; 
	bi.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_NEWDIALOGSTYLE; 
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi); 
	if (pidl != 0) 
	{ 
		char path[MAX_PATH]; 
		if (SHGetPathFromIDList(pidl, path)) 
		{ 
			std::string result(path); 
			CoTaskMemFree(pidl); 
			return result; 
		} 
		CoTaskMemFree(pidl); 
	} 
	return std::nullopt; 
}
// Function to open the Save File dialog and get the file path 
std::optional<std::wstring> ShowOpenSaveFileDialogue() { 
	 CoInitialize(NULL); 
	 IFileSaveDialog* pFileSave = NULL; 
	 HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, (void**)&pFileSave); 
	 if (SUCCEEDED(hr)) { 
		 hr = pFileSave->Show(NULL); 
		 if (SUCCEEDED(hr)) 
		 { 
			 IShellItem* pItem; 
			 hr = pFileSave->GetResult(&pItem); 
			 if (SUCCEEDED(hr)) 
			 { 
				 PWSTR pszFilePath; 
				 hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath); 
				 if (SUCCEEDED(hr)) 
				 { 
					 std::wstring filePath = pszFilePath; 
					 CoTaskMemFree(pszFilePath); 
					 pItem->Release(); 
					 pFileSave->Release(); 
					 CoUninitialize(); 
					 return filePath; 
				 } 
				 pItem->Release(); 
			 } 
		 } 
		 pFileSave->Release();
	 } 
	 CoUninitialize(); 
	 return std::nullopt;
}
// file_filter_map: key = file Type name, value = file extension (seperated by ';')
// returns: file path if user selected a file, std::nullopt if user cancelled
// example usage: std::optional<std::string> file_path = OpenFile("Select a file", { {"Text Files", "*.txt"}, {"All Files", "*.*"} });
std::optional<std::string> ShowOpenFileDialogue(const std::initializer_list<std::pair<std::string, std::string>>& file_filter_map, HWND parent = nullptr)
{
	OPENFILENAME ofn;
	//maximum length of a file name
	char szFile[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = parent;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	std::vector<char> file_filter_buffer;
	for (auto& [type, ext] : file_filter_map)
	{
		file_filter_buffer.insert(file_filter_buffer.end(), type.begin(), type.end());
		file_filter_buffer.push_back('\0');
		file_filter_buffer.insert(file_filter_buffer.end(), ext.begin(), ext.end());
		file_filter_buffer.push_back('\0');
	}
	//when the api detects 2 null characters it stops reading the string
	file_filter_buffer.push_back('\0');
	ofn.lpstrFilter = file_filter_buffer.data();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		return ofn.lpstrFile;
	}
	return std::nullopt;
}
void MarkPathInExplorer(const std::filesystem::path& p)
{
	auto pa = p.string();
	//ShellExecute(NULL, "open", pa.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
	ShellExecute(nullptr, "open", "explorer.exe", ("/select," + pa).c_str(), nullptr, SW_SHOWDEFAULT);
}
void OpenFolderInsideExplorer(const std::filesystem::path& p)
{
	auto pa = p.string();
	ShellExecute(nullptr, "open", pa.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
}

const std::string MEDIA_DIRECTORY = "./Resources/";

class CommonFireFLyeScene : public BaseScene
{
protected:
	RipeGrain::SceneObject background;
protected:
	RipeGrain::UILayer ui_layer;
	RipeGrain::ObjectsLayer background_layer;
public:
	void Initialize() override
	{
		SetViewPortSize(750, 650);
		background.AddSprite(CreateSprite(Image{ MEDIA_DIRECTORY + "fireflyebg.jpg" }));
		background.SetPosition(370, 250);
		background_layer.AddObject(&background);
		ui_layer.ui_clip_buffer = getCoreEngine().CreateStencilBuffer(750, 650);
		AddLayer(&background_layer);
		AddLayer(&ui_layer);
		RegisterEvent(RipeGrain::CreateUIRegisterEvent(&ui_layer));
	}
};

class FireFlyeScene : public CommonFireFLyeScene
{
public:
	void Initialize() override
	{
		CommonFireFLyeScene::Initialize();

		Image main_frame_img{ 500 , 350 };
		main_frame_img.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });

		auto main_frame = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 125,
				.position_y = 150,
				.ui_sprite = CreateSprite(main_frame_img)
			});

		Image create_btn_image(MEDIA_DIRECTORY + "icons8-create-100.png");
		auto create_btn = main_frame->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 50,
				.position_y = 50,
				.ui_sprite = CreateSprite(create_btn_image)
			});

		Image load_btn_image(MEDIA_DIRECTORY + "icons8-folder-100.png");
		auto load_btn = main_frame->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 200,
				.position_y = 50,
				.ui_sprite = CreateSprite(load_btn_image)
			});

		load_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					LoadScene<FireFLyeLoadedScene>();
				}
			};

		/*create_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					auto str = ShowOpenSaveFileDialogue();
					//if (str)
						//LoadScene<FireFLyeLoadedScene>(*str);
				}
			};*/
	}
};

class FireFLyeLoadedScene : public CommonFireFLyeScene
{
protected:
	Font cake_cafe_10;
	Font cake_cafe_12;
	Font cake_cafe_14;
	FontFamily cake_cafe;
private:
	ImageSprite tag_panel_sprite;
	ImageSprite record_file_panel_sprite;
	ImageSprite record_folder_panel_sprite;
private:
	ImageSprite panel_checkbox;
	Texture panel_checkbox_checked;
	Texture panel_checkbox_unchecked;
private:
	std::wstring search_string;
private:
	RipeGrain::UIComponent::UIPtr tag_search_result_area;
	RipeGrain::UIComponent::UIPtr record_action_bind_area;
	RipeGrain::UIComponent::UIPtr record_search_result_area;
	RipeGrain::UIComponent::UIPtr current_active_search_result_area;
private:
	typedef struct
	{
		int id;
		char* name;
		int len;
	} Record, Tag, FireData;
	typedef struct
	{
		FireData* data;
		int len;
	}RecordList, TagList, FireDataList;
private:
	typedef void(*fn_type_load_db)(const char*, int);
	typedef int (*fn_type_create_record)(int,const char*, int);
	typedef int (*fn_type_create_tag)(const char*, int);
	typedef RecordList(*fn_type_search_record_by_name)(int,const char*, int);
	typedef RecordList(*fn_type_search_record_by_tags)(int,int*, int);
	typedef TagList(*fn_type_search_tag_by_name)(const char*, int);
	typedef void (*fn_type_bind_record_to_tag)(int,int, int);
	typedef void (*fn_type_release_data_list)(FireDataList);
private:
	fn_type_load_db load_db;
	fn_type_create_tag create_tag;
	fn_type_create_record create_record;
	fn_type_release_data_list release_data_list;
	fn_type_search_tag_by_name search_tag_by_name;
	fn_type_bind_record_to_tag bind_record_to_tag;
	fn_type_search_record_by_name search_record_by_name;
	fn_type_search_record_by_tags search_record_by_tags;
private:
	HMODULE runtime_manager_module;
private:
	std::vector<int> selected_tags;
	std::vector<std::pair<int,int>> selected_records;
private:
	int db_panel_last_pos_y = 0;
private:
	RipeGrain::UIComponent::UIPtr db_frame;
private:
	class DbPanel
	{
	private:
		RipeGrain::UIComponent::UIPtr ui_ptr;
	private:
		Texture active_panel, inactive_panel;
	public:
		DbPanel(RipeGrain::UIComponent::UIPtr ptr , Texture active_p , Texture inactive_p) : ui_ptr(ptr) , active_panel(active_p) , inactive_panel(inactive_p){}
	public:
		void Activate()
		{
			ui_ptr->SetUITexture(active_panel);
		}
		void Inactivate()
		{
			ui_ptr->SetUITexture(inactive_panel);
		}
	};
private:
	int current_active_db_index = -1;
private:
	std::vector<std::string> loaded_databases;
	std::vector<DbPanel> loaded_database_panels;
public:
	FireFLyeLoadedScene() : cake_cafe(MEDIA_DIRECTORY + "fonts/Cakecafe.ttf")
	{
		cake_cafe_10 = Font{ cake_cafe , 10 };
		cake_cafe_12 = Font{ cake_cafe , 12 };
		cake_cafe_14 = Font{ cake_cafe , 14 };
		load_manager_runtime();
	}
	~FireFLyeLoadedScene()
	{
		FreeLibrary(runtime_manager_module);
	}
private:
	void load_manager_runtime()
	{
		runtime_manager_module = LoadLibrary("FlyeManager.dll");
		assert(runtime_manager_module != nullptr);

		load_db = (fn_type_load_db)GetProcAddress(runtime_manager_module, "load_db");
		create_tag = (fn_type_create_tag)GetProcAddress(runtime_manager_module, "create_tag");
		create_record = (fn_type_create_record)GetProcAddress(runtime_manager_module, "create_record");
		release_data_list = (fn_type_release_data_list)GetProcAddress(runtime_manager_module, "release_data_list");
		search_tag_by_name = (fn_type_search_tag_by_name)GetProcAddress(runtime_manager_module, "search_tag_by_name");
		bind_record_to_tag = (fn_type_bind_record_to_tag)GetProcAddress(runtime_manager_module, "bind_record_to_tag");
		search_record_by_name = (fn_type_search_record_by_name)GetProcAddress(runtime_manager_module, "search_record_by_name");
		search_record_by_tags = (fn_type_search_record_by_tags)GetProcAddress(runtime_manager_module, "search_record_by_tags");
	}
private:
	enum class PanelInteractionMode
	{
		Checked,
		Clicked,
		Unchecked,
	};
private:
	void AddPanel(const std::wstring& str, ImageSprite sprite, RipeGrain::UIComponent::UIPtr area, int x, int y , int width , int height , std::function<void(PanelInteractionMode)> callback)
	{
		Image text_img(width, height);
		text_img.Clear({ .a = 0 });

		text_img.DrawString(str, { .b = 100 , .g = 150 , .r = 25 }, 5, 15, cake_cafe_14);

		auto panel = area->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = x,
				.position_y = y,
				.ui_sprite = sprite
			});

		panel->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 40,
				.position_y = 5,
				.ui_sprite = CreateSprite(text_img)
			});

		auto cb = panel->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = (int)sprite.GetWidth() - 35,
				.position_y = 10,
				.ui_sprite = panel_checkbox
			});

		cb->on_mouse = [this , cb , checked = false , callback](RipeGrain::EventMouseInput ev) mutable
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					cb->SetUITexture( checked ? panel_checkbox_unchecked : panel_checkbox_checked);
					checked = !checked;
					callback(checked ? PanelInteractionMode::Checked : PanelInteractionMode::Unchecked);
				}
			};

		panel->on_mouse = [this, callback](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftDoublePress)
				{
					callback(PanelInteractionMode::Clicked);
				}
			};
	}
	void AddTagPanel(int id , const std::wstring& str, int x, int y)
	{
		AddPanel(str, tag_panel_sprite, tag_search_result_area, x, y, 125, 70, [this , id](PanelInteractionMode p_interact) 
			{
				switch (p_interact)
				{
				case PanelInteractionMode::Checked:
					selected_tags.push_back(id);
				break;
				case PanelInteractionMode::Unchecked:
					std::erase(selected_tags, id);
				break;
				}
			});
	}
	void AddRecordFilePanel(int manager_indx , int id , const std::wstring& str, int x, int y)
	{
		std::filesystem::path p = str;
		AddPanel(p.filename().wstring(), record_file_panel_sprite, record_search_result_area, x, y, 200, 70, [this, manager_indx,id, str](PanelInteractionMode p_interact)
			{
				switch (p_interact)
				{
				case PanelInteractionMode::Checked:
					selected_records.emplace_back(manager_indx,id);
					break;
				case PanelInteractionMode::Unchecked:
					std::erase(selected_records, std::make_pair(manager_indx, id));
					break;
				case PanelInteractionMode::Clicked:
					MarkPathInExplorer(str);
					break;
				}
			});
	}
	void AddRecordFolderPanel(int manager_indx,int id, const std::wstring& str, int x, int y)
	{
		std::filesystem::path p = str;
		AddPanel(p.filename().wstring(), record_folder_panel_sprite, record_search_result_area, x, y, 200, 70, [this,manager_indx, id, str](PanelInteractionMode p_interect)
			{
				switch (p_interect)
				{
				case PanelInteractionMode::Checked:
					selected_records.emplace_back(manager_indx,id);
					break;
				case PanelInteractionMode::Unchecked:
					std::erase(selected_records , std::make_pair(manager_indx, id));
					break;
				case PanelInteractionMode::Clicked:
					OpenFolderInsideExplorer(str);
					break;
				}
			});
	}
	void AddRecordPanel(int manager_indx,int id, const std::wstring& str, int x, int y)
	{
		if (std::filesystem::is_directory(str))
		{
			AddRecordFolderPanel(manager_indx,id,str, x, y);
		}
		else
		{
			AddRecordFilePanel(manager_indx,id,str, x, y);
		}
	}
public:
	void Initialize() override
	{
		CommonFireFLyeScene::Initialize();
		Image menu_frame_img(125, 350);
		menu_frame_img.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });

		auto menu = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 50,
				.position_y = 170,
				.ui_sprite = CreateSprite(menu_frame_img)
			});

		auto list_tag_btn = menu->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 10,
				.position_y = 5,
				.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY + "icons8-price-tag-100.png"})
			});
		auto list_record_btn = menu->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 10,
				.position_y = 110,
				.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY + "icons8-records-100.png"})
			});
		auto bind_record_btn = menu->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 10,
				.position_y = 215,
				.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY + "icons8-link-100.png"})
			});

		Image search_icon_img(MEDIA_DIRECTORY + "icons8-search-bar-48.png");
		Image search_area_img(300 , 70);
		search_area_img.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });
		search_area_img.DrawImage(search_icon_img , 5  , 0);

		auto search_bar = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 250,
				.position_y = 50,
				.ui_sprite = CreateSprite(search_area_img)
			});

		Image search_txt_img(250, 60);
		search_txt_img.Clear({ .a = 0 });

		auto search_area = search_bar->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 60,
				.position_y = 20,
				.ui_sprite = CreateSprite(search_txt_img)
			});

		search_bar->on_keyboard = [this , search_area ](RipeGrain::EventKeyBoardInput ev)
			{
				if (ev.type == RipeGrain::EventKeyBoardInput::Type::CharInput)
				{
					if (ev.key_code == VK_BACK)
					{
						if (!search_string.empty())
							search_string.pop_back();
					}
					else if (std::isprint(ev.key_code))
					{
						search_string += ev.key_code;
					}
					Image txt_img(250, 60);
					txt_img.Clear({ .a = 0 });
					txt_img.DrawString(search_string, { .b = 100 , .g = 150 , .r = 15 }, 10, 10, cake_cafe_12);
					search_area->SetUITexture(CreateTexture(txt_img));
				}
				else if (ev.type == RipeGrain::EventKeyBoardInput::Type::KeyPress && ev.key_code == VK_RETURN)
				{
					
					FireDataList fire_data_list;
					std::function<void(int , const std::wstring&, int, int)> create_panel;
					if (current_active_search_result_area == record_search_result_area)
					{
						auto str = convert_to_string(search_string);
						fire_data_list = search_record_by_name(current_active_db_index , str.c_str(), str.length());
						create_panel = std::bind_front(&FireFLyeLoadedScene::AddRecordPanel , this , current_active_db_index);
						selected_records.clear();
					}
					else if (current_active_search_result_area == tag_search_result_area)
					{
						auto str = convert_to_string(search_string);
						fire_data_list = search_tag_by_name(str.c_str(), str.length());
						create_panel = std::bind_front(&FireFLyeLoadedScene::AddTagPanel, this);
						selected_tags.clear();
					}
					else
					{
						return;
					}

					current_active_search_result_area->Clear();

					for (int i = 0; i < fire_data_list.len; ++i)
					{
						auto rec_name = std::string{ fire_data_list.data[i].name , fire_data_list.data[i].name + fire_data_list.data[i].len };
						create_panel(fire_data_list.data[i].id, convert_to_wstring(rec_name), 20, 70 * i + 10);
					}
					release_data_list(fire_data_list);
				}
			};
		Image search_result_area_image(450, 350);
		search_result_area_image.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });
		auto search_result_sprite = CreateSprite(search_result_area_image);
		
		record_search_result_area = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 200,
				.position_y = 150,
				.ui_sprite = search_result_sprite
			});
		tag_search_result_area = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 200,
				.position_y = 150,
				.ui_sprite = search_result_sprite
			});
		record_action_bind_area = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 200,
				.position_y = 150,
				.ui_sprite = search_result_sprite
			});

		current_active_search_result_area = tag_search_result_area;
		record_search_result_area->Hidden = true;
		record_action_bind_area->Hidden = true;

		auto scroll_page = [](RipeGrain::UIComponent::UIPtr panel, RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::Wheel)
				{
					panel->ScrollBy(ev.delta * 5);
				}
			};

		tag_search_result_area->on_mouse = std::bind_front(scroll_page , tag_search_result_area);
		record_search_result_area->on_mouse = std::bind_front(scroll_page, record_search_result_area);

		Image tag_panel_img(200, 60);
		tag_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });
		Image tag_img(MEDIA_DIRECTORY + "icons8-tags-35.png");

		tag_panel_img.DrawImage(tag_img, 0, 10);
		tag_panel_sprite = CreateSprite(tag_panel_img);

		Image record_file_panel_img(300, 60);
		record_file_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });

		Image record_folder_panel_img(300, 60);
		record_folder_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });

		Image folder_img(MEDIA_DIRECTORY + "icons8-mac-folder-35.png");
		Image file_img(MEDIA_DIRECTORY + "icons8-file-35.png");

		record_file_panel_img.DrawImage(file_img, 0, 10);
		record_folder_panel_img.DrawImage(folder_img, 0, 10);

		record_folder_panel_sprite = CreateSprite(record_folder_panel_img);
		record_file_panel_sprite = CreateSprite(record_file_panel_img);

		panel_checkbox_checked = CreateTexture(Image{ MEDIA_DIRECTORY + "icons8-checked-checkbox-30.png"});
		panel_checkbox_unchecked = CreateTexture(Image{ MEDIA_DIRECTORY + "icons8-unchecked-checkbox-30.png"});
		panel_checkbox = CreateSprite(panel_checkbox_unchecked);

		auto fn_change_active_panel = [this](RipeGrain::UIComponent::UIPtr panel_ptr, RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					current_active_search_result_area->Hidden = true;
					panel_ptr->Hidden = false;
					current_active_search_result_area = panel_ptr;
				}
			};

		list_tag_btn->on_mouse = std::bind_front(fn_change_active_panel, tag_search_result_area);
		bind_record_btn->on_mouse = std::bind_front(fn_change_active_panel, record_action_bind_area);
		list_record_btn->on_mouse = std::bind_front(fn_change_active_panel, record_search_result_area);

		Image img_bind_add_panel(200 , 150);
		img_bind_add_panel.Clear({ .b = 170 , .g = 255 , .r = 23 , .a = 150 });
		Image img_bind_add_icon(MEDIA_DIRECTORY + "icons8-add-70.png");

		img_bind_add_panel.DrawImage(img_bind_add_icon, 100, 10);
		img_bind_add_panel.DrawString(L"BIND SELECTED RECORDS", {.b = 4 , .g = 3 , .r = 15} , 10 , 120 , cake_cafe_10);

		auto bind_records_btn = record_action_bind_area->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 20,
				.position_y = 20,
				.ui_sprite = CreateSprite(img_bind_add_panel)
			});

		Image img_bind_search_panel(200, 150);
		img_bind_search_panel.Clear({ .b = 170 , .g = 255 , .r = 23 , .a = 150 });
		Image img_bind_search_icon(MEDIA_DIRECTORY + "icons8-search-70.png");

		img_bind_search_panel.DrawImage(img_bind_search_icon, 100, 10);
		img_bind_search_panel.DrawString(L"SEARCH BY SELECTED TAGS", { .b = 4 , .g = 3 , .r = 15 }, 10, 120, cake_cafe_10);

		auto bind_search_btn = record_action_bind_area->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 20,
				.position_y = 180,
				.ui_sprite = CreateSprite(img_bind_search_panel)
			});

		Image img_bind_global_search_panel(200, 150);
		img_bind_global_search_panel.Clear({ .b = 170 , .g = 255 , .r = 23 , .a = 150 });

		img_bind_global_search_panel.DrawImage(img_bind_search_icon, 100, 10);
		img_bind_global_search_panel.DrawString(L"GLOBAL SEARCH BY TAGS", { .b = 4 , .g = 3 , .r = 15 }, 10, 120, cake_cafe_10);

		auto bind_global_search_btn = record_action_bind_area->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 230,
				.position_y = 180,
				.ui_sprite = CreateSprite(img_bind_global_search_panel)
			});

		bind_search_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					auto fire_data_list = search_record_by_tags(current_active_db_index,selected_tags.data(), selected_tags.size());
					record_search_result_area->Clear();
					for (int i = 0; i < fire_data_list.len; ++i)
					{
						auto rec_name = std::string{ fire_data_list.data[i].name , fire_data_list.data[i].name + fire_data_list.data[i].len };
						AddRecordPanel(current_active_db_index,fire_data_list.data[i].id, convert_to_wstring(rec_name), 20, 70 * i + 10);
					}
					release_data_list(fire_data_list);
					current_active_search_result_area->Hidden = true;
					current_active_search_result_area = record_search_result_area;
					current_active_search_result_area->Hidden = false;
				}
			};
		bind_records_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					for (int tag_id : selected_tags)
					{
						for (auto[db_id,record_id] : selected_records)
						{
							bind_record_to_tag(db_id, record_id, tag_id);
						}
					}
				}
			};
		bind_global_search_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					record_search_result_area->Clear();
					int curr_h = 0;
					for (int db_i = 0; db_i < loaded_databases.size(); ++db_i)
					{
						auto fire_data_list = search_record_by_tags(db_i, selected_tags.data(), selected_tags.size());
						for (int i = 0; i < fire_data_list.len; ++i)
						{
							auto rec_name = std::string{ fire_data_list.data[i].name , fire_data_list.data[i].name + fire_data_list.data[i].len };
							AddRecordPanel(db_i,fire_data_list.data[i].id, convert_to_wstring(rec_name), 20, 70 * curr_h + 10);
							++curr_h;
						}
						release_data_list(fire_data_list);
					}
					current_active_search_result_area->Hidden = true;
					current_active_search_result_area = record_search_result_area;
					current_active_search_result_area->Hidden = false;
				}
			};
		auto add_new_record_btn = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 570,
				.position_y = 100,
				.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY + "icons8-add-row-48.png"})
			});
		add_new_record_btn->on_mouse = [this](RipeGrain::EventMouseInput ev) 
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					if (current_active_search_result_area == record_search_result_area)
					{
						auto path = ShowSelectFolderOrFileDialogue();
						if (path)
							create_record(current_active_db_index,path->c_str(), path->length());
					}
					else if (current_active_search_result_area == tag_search_result_area)
					{
						auto str = convert_to_string(search_string);
						create_tag(str.c_str(), str.length());
						auto s = "ADDED TAG: " + str;
						MessageBox(NULL, s.c_str(), "NEW TAG", MB_ICONEXCLAMATION);
					}

				}
			};

		Image db_collection_panel{ 170 , 100 };
		db_collection_panel.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });

		db_frame = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 60,
				.position_y = 40,
				.ui_sprite = CreateSprite(db_collection_panel)
			});
		db_frame->on_mouse = [this](RipeGrain::EventMouseInput ev) 
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::Wheel)
					db_frame->ScrollBy(ev.delta * 5);
			};

		auto load_db_btn = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
			{
				.position_x = 20,
				.position_y = 40,
				.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY + "icons8-add-to-database-40.png"})
			});

		load_db_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					auto p = ShowOpenSaveFileDialogue();
					if (p)
					{
						auto p_str = convert_to_string(*p);
						load_db(p_str.c_str(), p_str.length());
						CreateDbPanel(loaded_databases.size(), std::filesystem::path{ *p }.filename());
						loaded_databases.push_back(p_str);
					}
				}
			};
	}
private:
	void CreateDbPanel(int index , const std::wstring& name)
	{
		Image db_icon{ MEDIA_DIRECTORY + "icons8-registry-editor-35.png" };
		Image db_panel_img{ 150 , 45 };
		Image db_panel_active_img{ 150 , 45 };
		db_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 150 });
		db_panel_active_img.Clear({ .b = 170 , .g = 255 , .r = 23 , .a = 150 });

		db_panel_img.DrawImage(db_icon, 5, 0);
		db_panel_active_img.DrawImage(db_icon, 5, 0);

		db_panel_img.DrawString(name, {}, 45, 25, cake_cafe_12);
		db_panel_active_img.DrawString(name, {}, 45, 25, cake_cafe_12);

		auto db_panel_tex = CreateTexture(db_panel_img);
		auto db_panel_active_tex = CreateTexture(db_panel_active_img);

		auto panel = db_frame->AddComponent(RipeGrain::UIComponentDescription
			{
				.position_x = 10,
				.position_y = db_panel_last_pos_y + 10,
				.ui_sprite = CreateSprite(db_panel_tex)
			});
		db_panel_last_pos_y += 50;

		loaded_database_panels.emplace_back(panel,db_panel_active_tex ,db_panel_tex);

		panel->on_mouse = [index, this](RipeGrain::EventMouseInput ev)
			{
				if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
				{
					if (current_active_db_index != -1)
					{
						loaded_database_panels[current_active_db_index].Inactivate();
					}
					loaded_database_panels[index].Activate();
					current_active_db_index = index;
					record_search_result_area->Clear();
					selected_records.clear();
				}
			};
	}
};