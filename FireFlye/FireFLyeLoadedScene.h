#pragma once
#include "CommonFireFLyeScene.h"
#include "FireFLyePreviewEditorScene.h"
#include "FireFlyePreviewViewerScene.h"

class FireFlyePreviewEditorScene;

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
	ImageSprite preview_icon_sprite;
private:
	std::wstring search_string;
private:
	RipeGrain::UIComponent::UIPtr tag_search_result_area;
	RipeGrain::UIComponent::UIPtr record_action_bind_area;
	RipeGrain::UIComponent::UIPtr record_search_result_area;
	RipeGrain::UIComponent::UIPtr current_active_search_result_area;
private:
	std::vector<int> selected_tags;
	std::vector<std::pair<int, int>> selected_records;
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
		DbPanel(RipeGrain::UIComponent::UIPtr ptr, Texture active_p, Texture inactive_p) : ui_ptr(ptr), active_panel(active_p), inactive_panel(inactive_p) {}
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
private:
	RecordManagerModule manager_module;
public:
	FireFLyeLoadedScene() : FireFLyeLoadedScene(RecordManagerModule{}) {}
    FireFLyeLoadedScene(RecordManagerModule&& module) : manager_module(std::move(module)), cake_cafe(MEDIA_DIRECTORY / "fonts/Cakecafe.ttf")
    {  
		cake_cafe_10 = Font{ cake_cafe , 10 };
		cake_cafe_12 = Font{ cake_cafe , 12 };
		cake_cafe_14 = Font{ cake_cafe , 14 };
    }
	~FireFLyeLoadedScene() = default;
private:
	enum class PanelInteractionMode
	{
		Checked,
		LeftClicked,
		LeftDoubleClicked,
		RightClicked,
		RightDoubleClicked,
		Unchecked,
		PreviewClicked
	};
private:
	void AddTagPanel(int id, const std::wstring& str, int x, int y);
	void AddRecordPanel(int manager_indx, int id, const std::wstring& str, int x, int y);
	void AddRecordFilePanel(int manager_indx, int id, const std::wstring& str, int x, int y);
	void AddRecordFolderPanel(int manager_indx, int id, const std::wstring& str, int x, int y);
	void AddPanel(const std::wstring& str, ImageSprite sprite, RipeGrain::UIComponent::UIPtr area, int x, int y, int width, int height, std::function<void(PanelInteractionMode)> callback);
	void AddRecordPanelOptions(RipeGrain::UIComponent::UIPtr panel_component , std::function<void(PanelInteractionMode)> callback);
private:
	void AddRecordPanelNext(int manager_index, int id, const std::filesystem::path& full_path, int x, int y);
	void AddRecordPanelWithPreview(int manager_index, int id, Image preview , const std::filesystem::path& full_path, int x, int y);
public:
	void Initialize() override;
private:
	void CreateDbPanel(int index, const std::wstring& name);
private:
	void ActivateDbPanel(int index);
private:
	void change_active_panel(RipeGrain::UIComponent::UIPtr panel_ptr);
private:
	void on_record_search_by_name(const std::string& keyword);
	void on_tag_search_by_name(const std::string& keyword);
private:
	void on_record_panel_interaction(int manager_index, int record_id, const std::filesystem::path& path, PanelInteractionMode in_mode);
};