#include "FireFlyeScene.h"

class DummyScene : public BaseScene
{
private:
	RipeGrain::SceneObject object;
	RipeGrain::ObjectsLayer objects_layer;
public:
	void Initialize() override
	{
		auto s = CreateSprite(Image{100 , 100});
		object.AddSprite(s);
		object.SetPosition(s.GetHeight() / 2, s.GetWidth() / 2);
		objects_layer.AddObject(&object);
		AddLayer(&objects_layer);
		SetViewPortSize(400, 400);
	}
	void Update() override
	{
		Scene::Update();
	}
	void OnEventReceive(RipeGrain::Event& ev) override
	{
		if (ev.event_type_index == typeid(RipeGrain::EventKeyBoardInput))
		{
			auto ev_ = RipeGrain::GetEventData<RipeGrain::EventKeyBoardInput>(ev);
			switch (ev_.key_code)
			{
			case VK_SPACE:
				//LoadScene<>();
				break;
			}
		}
	}
};

// define the function to load the scene
void* GetScene()
{
	return new FireFlyeScene();
}

void UnloadScene(void* scene)
{
	delete static_cast<RipeGrain::Scene*>(scene);
}