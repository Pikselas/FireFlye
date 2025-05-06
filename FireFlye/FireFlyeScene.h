#pragma once
#include "CommonFireFlyeScene.h"
#include "FireFLyeLoadedScene.h"

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