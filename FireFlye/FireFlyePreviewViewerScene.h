#pragma once
#include "CommonFireFlyeScene.h"

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
public:
	FireFlyePreviewViewerScene(RecordManagerModule&& module, int manager_index, int record_id)
		:
		manager_module(std::move(module)),
		manager_index(manager_index),
		record_id(record_id)
	{
	}
	void Initialize() override
	{
		try
		{
			CommonFireFLyeScene::Initialize();
			auto previews = manager_module.get_previews_by_record_id(manager_index, record_id);
			for (int i = 0; i < previews.len; ++i)
			{
				auto preview_sprite = CreateSprite(Image{ std::span<char>(previews.data[i].preview , previews.data[i].preview_len)});
				ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
					{
						.position_x = 10,
						.position_y = (int)(preview_sprite.GetHeight() * i) + 10,
						.ui_sprite = preview_sprite
					});
				max_size += preview_sprite.GetHeight() + 10;
			}
			manager_module.release_record_preview_list(previews);

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
			auto delta = RipeGrain::GetEventData<RipeGrain::EventMouseInput>(ev).delta;
			ui_layer.Scroll(DirectX::XMVectorSet(0, delta * 5, 0, 0) , DirectX::XMVectorSet(0 , max_size , 0 , 1));
		}
	}
};