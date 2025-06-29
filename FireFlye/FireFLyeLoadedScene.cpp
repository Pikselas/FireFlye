#include "FireFLyeLoadedScene.h"
#include <iostream>

void FireFLyeLoadedScene::AddPanel(const std::wstring& str, ImageSprite sprite, RipeGrain::UIComponent::UIPtr area, int x, int y, int width, int height, std::function<void(PanelInteractionMode)> callback)
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

	cb->on_mouse = [this, cb, checked = false, callback](RipeGrain::EventMouseInput ev) mutable
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				cb->SetUITexture(checked ? panel_checkbox_unchecked : panel_checkbox_checked);
				checked = !checked;
				callback(checked ? PanelInteractionMode::Checked : PanelInteractionMode::Unchecked);
			}
		};

	panel->on_mouse = [this, callback](RipeGrain::EventMouseInput ev)
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftDoublePress)
			{
				callback(PanelInteractionMode::LeftDoubleClicked);
			}
			else if (ev.type == RipeGrain::EventMouseInput::Type::RightDoublePress)
			{
				callback(PanelInteractionMode::RightDoubleClicked);
			}
		};
}

void FireFLyeLoadedScene::AddRecordPanelOptions(RipeGrain::UIComponent::UIPtr panel_component , std::function<void(PanelInteractionMode)> callback)
{
	auto chk_bx = panel_component->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = 115,
			.position_y = 5,
			.ui_sprite = panel_checkbox
		});

	chk_bx->on_mouse = [chk_bx, checked = false , callback , this](RipeGrain::EventMouseInput ev) mutable
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				if (!checked)
				{
					callback(PanelInteractionMode::Checked);
				}
				else
				{
					callback(PanelInteractionMode::Unchecked);
				}

				chk_bx->SetUITexture(checked ? panel_checkbox_unchecked : panel_checkbox_checked);
				checked = !checked;
			}
		};

	auto preview_icon = panel_component->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = 115,
			.position_y = 40,
			.ui_sprite = preview_icon_sprite
		});

	preview_icon->on_mouse = [callback](RipeGrain::EventMouseInput ev)
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				callback(PanelInteractionMode::PreviewClicked);
			}
		};

	panel_component->on_mouse = [callback](RipeGrain::EventMouseInput ev)
		{
			switch (ev.type)
			{
			case RipeGrain::EventMouseInput::Type::LeftPress:
				callback(PanelInteractionMode::LeftClicked);
				break;
			case RipeGrain::EventMouseInput::Type::LeftDoublePress:
				callback(PanelInteractionMode::LeftDoubleClicked);
				break;
			case RipeGrain::EventMouseInput::Type::RightPress:
				callback(PanelInteractionMode::RightClicked);
				break;
			}
		};


}

void FireFLyeLoadedScene::AddRecordPanelNext(int manager_index, int id, const std::filesystem::path& full_path, int x, int y)
{
	Image panel_img{ 150 ,200 };
	panel_img.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });

	panel_img.DrawString(full_path.filename(), { .b = 100 , .g = 150 , .r = 25 }, 5, 150, cake_cafe_12);
	auto record_panel = record_search_result_area->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = x,
			.position_y = y,
			.ui_sprite = CreateSprite(panel_img)
		});
	record_panel->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = 10,
			.position_y = 10,
			.ui_sprite = std::filesystem::is_directory(full_path) ? record_folder_panel_sprite : record_file_panel_sprite
		});
	AddRecordPanelOptions(record_panel , std::bind_front(&FireFLyeLoadedScene::on_record_panel_interaction , this , manager_index , id , full_path));
}

void FireFLyeLoadedScene::AddRecordPanelWithPreview(int manager_index , int id, Image preview, const std::filesystem::path& full_path, int x, int y)
{
	Image panel_img{ 150 ,200 };
	panel_img.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });

	/*auto aspect_ratio = (float)preview.GetWidth() / (float)preview.GetHeight();

	if (preview.GetHeight() > preview.GetWidth())
		preview.Resize((int)(150 * aspect_ratio), 150);
	else
		preview.Resize(150, (int)(150 / aspect_ratio));*/

	resize_frame(preview, 150);

	panel_img.DrawImage(preview, std::abs(150 - (int)preview.GetWidth()) / 2, std::abs(150 - (int)preview.GetHeight()) / 2);
	panel_img.DrawString(full_path.filename().wstring(), { .b = 100 , .g = 150 , .r = 25 }, 5, 150, cake_cafe_14);


	auto record_panel = record_search_result_area->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = x,
			.position_y = y,
			.ui_sprite = CreateSprite(panel_img)
		});

	AddRecordPanelOptions(record_panel, std::bind_front(&FireFLyeLoadedScene::on_record_panel_interaction, this, manager_index, id, full_path));
}

void FireFLyeLoadedScene::AddTagPanel(int id, const std::wstring& str, int x, int y)
{
	AddPanel(str, tag_panel_sprite, tag_search_result_area, x, y, 125, 70, [this, id](PanelInteractionMode p_interact)
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

void FireFLyeLoadedScene::AddRecordFilePanel(int manager_indx, int id, const std::wstring& str, int x, int y)
{
	std::filesystem::path p = str;
	AddPanel(p.filename().wstring(), record_file_panel_sprite, record_search_result_area, x, y, 200, 70, [this, manager_indx, id, str](PanelInteractionMode p_interact)
		{
			switch (p_interact)
			{
			case PanelInteractionMode::Checked:
				selected_records.emplace_back(manager_indx, id);
				break;
			case PanelInteractionMode::Unchecked:
				std::erase(selected_records, std::make_pair(manager_indx, id));
				break;
			case PanelInteractionMode::LeftClicked:
				//MarkPathInExplorer(str);
				//LoadScene<FireFlyePreviewEditorScene>(std::move(RecordManagerModule::GetInstance()), current_active_db_index , id , str);
				//LoadScene<FireFlyePreviewViewerScene>(std::move(RecordManagerModule::GetInstance()), current_active_db_index, id);
				break;
			case PanelInteractionMode::RightClicked:
				/*LoadScene<FireFlyePreviewEditorScene>(loaded_databases[current_active_db_index] , str);*/
				break;
			}
		});
}

void FireFLyeLoadedScene::AddRecordFolderPanel(int manager_indx, int id, const std::wstring& str, int x, int y)
{
	std::filesystem::path p = str;
	AddPanel(p.filename().wstring(), record_folder_panel_sprite, record_search_result_area, x, y, 200, 70, [this, manager_indx, id, str](PanelInteractionMode p_interect)
		{
			switch (p_interect)
			{
			case PanelInteractionMode::Checked:
				selected_records.emplace_back(manager_indx, id);
				break;
			case PanelInteractionMode::Unchecked:
				std::erase(selected_records, std::make_pair(manager_indx, id));
				break;
			case PanelInteractionMode::LeftClicked:
				OpenFolderInsideExplorer(str);
				break;
			}
		});
}

void FireFLyeLoadedScene::AddRecordPanel(int manager_indx, int id, const std::wstring& str, int x, int y)
{
	if (std::filesystem::is_directory(str))
	{
		AddRecordFolderPanel(manager_indx, id, str, x, y);
	}
	else
	{
		AddRecordFilePanel(manager_indx, id, str, x, y);
	}
}

void FireFLyeLoadedScene::Initialize()
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
			.ui_sprite = CreateSprite(Image{ MEDIA_DIRECTORY / "icons8-price-tag-100.png"})
		});
	auto list_record_btn = menu->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = 10,
			.position_y = 110,
			.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY / "icons8-records-100.png"})
		});
	auto bind_record_btn = menu->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = 10,
			.position_y = 215,
			.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY / "icons8-link-100.png"})
		});

	Image search_icon_img(MEDIA_DIRECTORY / "icons8-search-bar-48.png");
	Image search_area_img(300, 70);
	search_area_img.Clear({ .b = 18 , .g = 13 , .r = 5 , .a = 150 });
	search_area_img.DrawImage(search_icon_img, 5, 0);

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

	search_bar->on_keyboard = [this, search_area](RipeGrain::EventKeyBoardInput ev)
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

				current_active_search_result_area->Clear();
				if (current_active_search_result_area == record_search_result_area)
				{
					/*fire_data_list = RecordManagerModule::GetInstance().search_record_by_name(current_active_db_index, str.c_str(), str.length());
					create_panel = std::bind_front(&FireFLyeLoadedScene::AddRecordPanel, this, current_active_db_index);
					selected_records.clear();*/
					on_record_search_by_name(convert_to_string(search_string));
					
				}
				else if (current_active_search_result_area == tag_search_result_area)
				{
					on_tag_search_by_name(convert_to_string(search_string));
				}
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

	tag_search_result_area->on_mouse = std::bind_front(scroll_page, tag_search_result_area);
	record_search_result_area->on_mouse = std::bind_front(scroll_page, record_search_result_area);

	Image tag_panel_img(200, 60);
	tag_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });
	Image tag_img(MEDIA_DIRECTORY / "icons8-tags-35.png");

	tag_panel_img.DrawImage(tag_img, 0, 10);
	tag_panel_sprite = CreateSprite(tag_panel_img);

	/*Image record_file_panel_img(300, 60);
	record_file_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });

	Image record_folder_panel_img(300, 60);
	record_folder_panel_img.Clear({ .b = 20 , .g = 20 , .r = 15 , .a = 200 });*/

	Image folder_img(MEDIA_DIRECTORY / "icons8-mac-folder-35.png");
	Image file_img(MEDIA_DIRECTORY / "icons8-file-35.png");

	//record_file_panel_img.DrawImage(file_img, 0, 10);
	//record_folder_panel_img.DrawImage(folder_img, 0, 10);

	/*record_folder_panel_sprite = CreateSprite(record_folder_panel_img);
	record_file_panel_sprite = CreateSprite(record_file_panel_img);*/

	record_folder_panel_sprite = CreateSprite(folder_img);
	record_file_panel_sprite = CreateSprite(file_img);

	panel_checkbox_checked = CreateTexture(Image{ MEDIA_DIRECTORY / "icons8-checked-checkbox-30.png" });
	panel_checkbox_unchecked = CreateTexture(Image{ MEDIA_DIRECTORY / "icons8-unchecked-checkbox-30.png" });
	panel_checkbox = CreateSprite(panel_checkbox_unchecked);

	preview_icon_sprite = CreateSprite(Image{ MEDIA_DIRECTORY / "icons8-preview-30.png" });

	auto fn_change_active_panel = [this](RipeGrain::UIComponent::UIPtr target , RipeGrain::EventMouseInput ev)
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				change_active_panel(target);
			}
		};

	list_tag_btn->on_mouse = std::bind_front(fn_change_active_panel, tag_search_result_area);
	bind_record_btn->on_mouse = std::bind_front(fn_change_active_panel, record_action_bind_area);
	list_record_btn->on_mouse = std::bind_front(fn_change_active_panel, record_search_result_area);

	Image img_bind_add_panel(200, 150);
	img_bind_add_panel.Clear({ .b = 170 , .g = 255 , .r = 23 , .a = 150 });
	Image img_bind_add_icon(MEDIA_DIRECTORY / "icons8-add-70.png");

	img_bind_add_panel.DrawImage(img_bind_add_icon, 100, 10);
	img_bind_add_panel.DrawString(L"BIND SELECTED RECORDS", { .b = 4 , .g = 3 , .r = 15 }, 10, 120, cake_cafe_10);

	auto bind_records_btn = record_action_bind_area->AddComponent(RipeGrain::UIComponentDescription
		{
			.position_x = 20,
			.position_y = 20,
			.ui_sprite = CreateSprite(img_bind_add_panel)
		});

	Image img_bind_search_panel(200, 150);
	img_bind_search_panel.Clear({ .b = 170 , .g = 255 , .r = 23 , .a = 150 });
	Image img_bind_search_icon(MEDIA_DIRECTORY / "icons8-search-70.png");

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
				auto fire_data_list = RecordManagerModule::GetInstance().search_record_by_tags(current_active_db_index, selected_tags.data(), selected_tags.size());
				record_search_result_area->Clear();
				for (int i = 0; i < fire_data_list.len; ++i)
				{
					auto rec_name = std::string{ fire_data_list.data[i].name , fire_data_list.data[i].name + fire_data_list.data[i].len };
					AddRecordPanel(current_active_db_index, fire_data_list.data[i].id, convert_to_wstring(rec_name), 20, 70 * i + 10);
				}
				RecordManagerModule::GetInstance().release_data_list(fire_data_list);
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
					for (auto [db_id, record_id] : selected_records)
					{
						RecordManagerModule::GetInstance().bind_record_to_tag(db_id, record_id, tag_id);
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
					auto fire_data_list = RecordManagerModule::GetInstance().search_record_by_tags(db_i, selected_tags.data(), selected_tags.size());
					for (int i = 0; i < fire_data_list.len; ++i)
					{
						auto rec_name = std::string{ fire_data_list.data[i].name , fire_data_list.data[i].name + fire_data_list.data[i].len };
						AddRecordPanel(db_i, fire_data_list.data[i].id, convert_to_wstring(rec_name), 20, 70 * curr_h + 10);
						++curr_h;
					}
					RecordManagerModule::GetInstance().release_data_list(fire_data_list);
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
			.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY / "icons8-add-row-48.png"})
		});
	add_new_record_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				if (current_active_search_result_area == record_search_result_area)
				{
					auto path = ShowSelectFolderOrFileDialogue();
					if (path)
						RecordManagerModule::GetInstance().create_record(current_active_db_index, path->c_str(), path->length());
				}
				else if (current_active_search_result_area == tag_search_result_area)
				{
					auto str = convert_to_string(search_string);
					RecordManagerModule::GetInstance().create_tag(str.c_str(), str.length());
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
			.ui_sprite = CreateSprite(Image{MEDIA_DIRECTORY / "icons8-add-to-database-40.png"})
		});

	load_db_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				auto p = ShowOpenSaveFileDialogue();
				if (p)
				{
					auto p_str = convert_to_string(*p);
					RecordManagerModule::GetInstance().load_db(p_str.c_str(), p_str.length());
					CreateDbPanel(loaded_databases.size(), std::filesystem::path{ *p }.filename());
					loaded_databases.push_back(p_str);
				}
			}
		};

	auto managers = RecordManagerModule::GetInstance().get_record_manager_list();
	for (int i = 0; i < managers.len; ++i)
	{
		auto db_path = std::filesystem::path{ managers.data[i].name , managers.data[i].name + managers.data[i].len };
		CreateDbPanel(i, db_path.filename());
		loaded_databases.emplace_back(db_path.string());
	}
	RecordManagerModule::GetInstance().release_record_manager_list(managers);
}

void FireFLyeLoadedScene::CreateDbPanel(int index, const std::wstring& name)
{
	Image db_icon{ MEDIA_DIRECTORY / "icons8-registry-editor-35.png" };
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

	loaded_database_panels.emplace_back(panel, db_panel_active_tex, db_panel_tex);

	panel->on_mouse = [index, this](RipeGrain::EventMouseInput ev)
		{
			if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
			{
				ActivateDbPanel(index);
			}
		};
}

void FireFLyeLoadedScene::ActivateDbPanel(int index)
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

void FireFLyeLoadedScene::change_active_panel(RipeGrain::UIComponent::UIPtr panel_ptr)
{
	current_active_search_result_area->Hidden = true;
	panel_ptr->Hidden = false;
	current_active_search_result_area = panel_ptr;
}

void FireFLyeLoadedScene::on_record_search_by_name(const std::string& keyword)
{
	selected_records.clear();
	auto data_list = RecordManagerModule::GetInstance().search_record_with_preview_by_name(current_active_db_index, keyword.c_str(), keyword.length());
	for (int i = 0; i < data_list.len; ++i)
	{
		auto rec_path = std::filesystem::path(data_list.data[i].name , data_list.data[i].name + data_list.data[i].len);
		ImageSprite prev;
		if (data_list.data[i].preview_len != 0)
		{
			AddRecordPanelWithPreview(current_active_db_index , data_list.data[i].id, Image(std::span<char>(data_list.data[i].preview, data_list.data[i].preview_len)), rec_path, 10, i * 210);
		}
		else
		{
			AddRecordPanelNext(current_active_db_index , data_list.data[i].id, rec_path, 10, i * 210);
		}
	}
	RecordManagerModule::GetInstance().release_record_preview_list(data_list);
}

void FireFLyeLoadedScene::on_tag_search_by_name(const std::string& keyword)
{
	selected_tags.clear();
	RecordManagerModule::FireDataList fire_data_list;
	fire_data_list = RecordManagerModule::GetInstance().search_tag_by_name(keyword.c_str(), keyword.length());
	for (int i = 0; i < fire_data_list.len; ++i)
	{
		auto rec_name = std::string{ fire_data_list.data[i].name , fire_data_list.data[i].name + fire_data_list.data[i].len };
		AddTagPanel(fire_data_list.data[i].id, convert_to_wstring(rec_name), 20, 70 * i + 10);
	}
	RecordManagerModule::GetInstance().release_data_list(fire_data_list);
}

void FireFLyeLoadedScene::on_record_panel_interaction(int manager_index, int record_id, const std::filesystem::path& path, PanelInteractionMode in_mode)
{
	switch (in_mode)
	{
	case PanelInteractionMode::Checked:
		selected_records.push_back(std::make_pair(manager_index, record_id));
		break;
	case PanelInteractionMode::Unchecked:
		std::erase(selected_records, std::make_pair(manager_index, record_id));
		break;
	case PanelInteractionMode::LeftDoubleClicked:
		if(std::filesystem::is_directory(path))
			OpenFolderInsideExplorer(path);
		else
			MarkPathInExplorer(path);
		break;
	case PanelInteractionMode::RightClicked:
		break;
	case PanelInteractionMode::PreviewClicked:
		LoadScene<FireFlyePreviewViewerScene>(manager_index, record_id , path);
		break;
	default:
		break;
	}
}
