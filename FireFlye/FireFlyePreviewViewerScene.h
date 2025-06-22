#pragma once
#include "CommonFireFlyeScene.h"
#include "FireFlyePreviewEditorScene.h"

class FireFLyeLoadedScene;

class FireFlyePreviewViewerScene : public CommonFireFLyeScene
{
private:
	RecordManagerModule manager_module;
private:
	const int manager_index;
	const int record_id;
private:
	int max_size = 0;
private:
	RipeGrain::UIComponent::UIPtr preview_layer;
private:
	const std::filesystem::path record_path;
public:
	FireFlyePreviewViewerScene(RecordManagerModule&& module, int manager_index, int record_id , const std::filesystem::path& record_path)
		:
		manager_module(std::move(module)),
		manager_index(manager_index),
		record_id(record_id),
		record_path(record_path)
	{
	}
	void Initialize() override
	{
		try
		{
			CommonFireFLyeScene::Initialize();
			preview_layer = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 10,
					.position_y = 10,
					.ui_sprite = CreateSprite(Image{ 500, 400 })
				});

			auto add_preview_btn = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 520,
					.position_y = 25,
					.ui_sprite = CreateSprite(Image{ MEDIA_DIRECTORY / "icons8-add-70.png" })
				});

			auto previews = manager_module.get_previews_by_record_id(manager_index, record_id);
			for (int i = 0; i < previews.len; ++i)
			{
				auto preview_sprite = CreateSprite(Image{ std::span<char>(previews.data[i].preview , previews.data[i].preview_len)});
				preview_layer->AddComponent(RipeGrain::UIComponentDescription
					{
						.position_x = 10,
						.position_y = (int)(preview_sprite.GetHeight() * i) + 10,
						.ui_sprite = preview_sprite
					});
				max_size += preview_sprite.GetHeight() + 10;
			}
			manager_module.release_record_preview_list(previews);

			add_preview_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
				{
					if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
					{
						if (!std::filesystem::is_directory(record_path))
							LoadScene<FireFlyePreviewEditorScene>(std::move(manager_module), manager_index, record_id, record_path);
						else
						{
							if (auto file = ShowOpenFileDialogue({}, record_path.string().c_str()))
							{
								LoadScene<FireFlyePreviewEditorScene>(std::move(manager_module), manager_index, record_id, std::filesystem::path(*file));
							}
						}
					}
				};

			//preview_layer.AddObject();
		}
		catch (...)
		{

		}
	}

	void OnEventReceive(RipeGrain::Event& ev) override
	{
		if (ev.event_type_index == typeid(RipeGrain::EventMouseInput))
		{
			auto mouse_ev = RipeGrain::GetEventData<RipeGrain::EventMouseInput>(ev);
			switch (mouse_ev.type)
			{
			case RipeGrain::EventMouseInput::Type::Wheel:
				auto delta = mouse_ev.delta;
				preview_layer->ScrollBy(delta * 5);
					//Scroll(DirectX::XMVectorSet(0, delta * 5, 0, 0), DirectX::XMVectorSet(0, max_size, 0, 1));
			break;
			}
		}
	}
};