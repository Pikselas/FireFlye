#pragma once
#include "CommonFireFlyeScene.h"

class FireFLyeLoadedScene;

class FireFlyePreviewEditorScene : public CommonFireFLyeScene
{
private:
	bool is_knob_active = false;
private:
	std::filesystem::path path;
private:
	VideoReader reader;
private:
	RipeGrain::ObjectsLayer video_layer;
private:
	RipeGrain::SceneObject video_frame_obj;
private:
	Image current_active_frame;
	std::vector<Image> video_frames;
private:
	const int manager_index;
	const int record_id;
public:
	FireFlyePreviewEditorScene(int manager_index, int record_id, const std::filesystem::path& path)
		:
		current_active_frame(100 , 100),
		manager_index(manager_index),
		record_id(record_id),
		path(path)
	{
		//load_db(record_path.c_str(), record_path.length());
	}
private:
	void Initialize() override
	{
		try
		{
			CommonFireFLyeScene::Initialize();
			AddLayer(&video_layer);
			video_layer.AddObject(&video_frame_obj);

			reader.Open(path);
			current_active_frame = reader.GetNextFrame()->GetImage();
			resize_frame(current_active_frame , 300);
			auto video_frame_sprite = CreateSprite(current_active_frame);
			//auto scale_factor_x = 300.0f / (float)current_active_frame.GetWidth();

			video_frame_sprite.SetPosition(DirectX::XMVectorSet(200, 250, 0, 0));
			//video_frame_sprite.SetTransformation(DirectX::XMMatrixScaling(scale_factor_x, scale_factor_x, 1.0f));

			video_frame_obj.AddSprite(video_frame_sprite);

			/*auto panel = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 50,
					.position_y = 0,
					.ui_sprite = frame_sprite
				});*/

			Image seek_bar{ 350 , 10 };
			seek_bar.Clear({ .b = 10 , .g = 10 , .r = 10 });

			Image seek_knob{ MEDIA_DIRECTORY / "icons8-slider-20.png" };

			auto seek = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 50,
					.position_y = 550,
					.ui_sprite = CreateSprite(seek_bar)
				});

			auto knob = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 50,
					.position_y = 545,
					.ui_sprite = CreateSprite(seek_knob)
				});

			seek->on_mouse = [seek, knob, this](RipeGrain::EventMouseInput ev)
				{
					switch (ev.type)
					{
					case RipeGrain::EventMouseInput::Type::LeftPress:
						is_knob_active = true;
						break;
					case RipeGrain::EventMouseInput::Type::LeftRelease:
						is_knob_active = false;
						break;
					case RipeGrain::EventMouseInput::Type::Move:
						if (is_knob_active)
						{
							auto dur = reader.GetDuration() * ((float)ev.client_x / seek->GetWidth());
							dur = std::clamp((unsigned long long)dur, (unsigned long long)0, reader.GetDuration());
							try
							{
								knob->SetX(ev.client_x - knob->GetWidth() / 2);
								reader.SeekTo(dur);
								auto frame = reader.GetNextFrame();
								if (frame)
								{
									current_active_frame = std::move(frame->GetImage());
									resize_frame(current_active_frame, 300);
									video_frame_obj.GetSprites().front().SetTexture(CreateTexture(current_active_frame));
									//panel->SetUITexture(CreateTexture(frame->GetImage()));
								}
							}
							catch (std::exception& e)
							{
								MessageBox(nullptr, e.what(), "ERROR", MB_ICONERROR);
							}
						}
						break;
					}
				};

			Image captured_frame{ 450 , 450 };
			captured_frame.Clear({ .b = 10 , .g = 10 , .r = 10 , .a = 200 });
			auto captured_frame_panel = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 400,
					.position_y = 20,
					.ui_sprite = CreateSprite(captured_frame)
				});

			captured_frame_panel->on_mouse = [captured_frame_panel](RipeGrain::EventMouseInput ev)
				{
					if (ev.type == RipeGrain::EventMouseInput::Type::Wheel)
					{
						captured_frame_panel->ScrollBy(ev.delta * 5);
					}
				};

			Image capture_img{ MEDIA_DIRECTORY / "icons8-camera-48.png" };
			auto capture_btn = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 425,
					.position_y = 550,
					.ui_sprite = CreateSprite(capture_img)
				});

			capture_btn->on_mouse = [captured_frame_panel, this, scroll_height = 0](RipeGrain::EventMouseInput ev) mutable
				{
					if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
					{
						auto frame = video_frame_obj.GetSprites().front();
						auto scale_factor = 150.0f / (float)frame.GetWidth();
						frame.SetTransformation(DirectX::XMMatrixScaling(scale_factor, scale_factor, 1.0f));
						captured_frame_panel->AddComponent(RipeGrain::UIComponentDescription
							{
								.position_x = 5,
								.position_y = scroll_height,
								.ui_sprite = frame
							});
						scroll_height += (frame.GetHeight() * scale_factor) + 10;
						captured_frame_panel->ScrollBy(-scroll_height);
						video_frames.emplace_back(current_active_frame);
					}
				};

			Image save_img{ MEDIA_DIRECTORY / "icons8-save-48.png" };
			auto save_btn = ui_layer.AddUIFrame(RipeGrain::UIComponentDescription
				{
					.position_x = 500,
					.position_y = 550,
					.ui_sprite = CreateSprite(save_img)
				});

			save_btn->on_mouse = [this](RipeGrain::EventMouseInput ev)
				{
					if (ev.type == RipeGrain::EventMouseInput::Type::LeftPress)
					{
						for (auto& frame : video_frames)
						{
							auto img_buffer = frame.SaveToBuffer(".jpg");
							RecordManagerModule::GetInstance().add_preview(manager_index, record_id, img_buffer.data(), img_buffer.size());
						}
						MessageBox(nullptr, "Frames saved", "SUCCESS", MB_ICONINFORMATION);
						LoadScene<FireFLyeLoadedScene>();
					}
				};

		}
		catch (const std::exception& e)
		{
			MessageBox(nullptr, e.what(), "FAILED", MB_ICONERROR);
			LoadScene<FireFLyeLoadedScene>();
		}
	}
};