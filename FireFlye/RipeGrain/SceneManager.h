#pragma once
#include <list>
#include <utility>
#include "SceneLayer.h"
#include "SceneObject.h"
#include "ObjectAnimator.h"
#include "EngineComponent.h"
#include "RepulsiveEngine/ResourceEngine.h"

namespace RipeGrain
{
	class Scene;

	template<typename T>
	concept CScene = std::is_base_of_v<Scene, T>;

	template<typename T>
	concept CSceneObject = std::is_base_of_v<SceneObject,T>;

	class SceneLoader : public EngineEventRaiser
	{
	public:
		template<CScene T, typename... ParamsT>
		void LoadScene(ParamsT&& ... params , std::function<void(Scene*)> deleter = [](Scene* s) {delete s; })
		{
			auto current_scene = new T(std::forward<ParamsT>(params)...);
			current_scene->SetSceneLoader(this);
			auto scene_event = CreateEventObject(std::move(EventSceneLoaded{ current_scene , deleter }));
			RaiseEvent(std::move(scene_event));
		}
		template<CScene T>
		void LoadScene(T* scene , std::function<void(Scene*)> deleter)
		{
			auto scene_event = CreateEventObject(std::move(EventSceneLoaded{ scene , deleter }));
			scene->SetSceneLoader(this);
			RaiseEvent(std::move(scene_event));
		}
	};

	class Scene
	{
	private:
		unsigned int view_port_width;
		unsigned int view_port_height;
	private:
		ResourceEngine* sprite_engine = nullptr;
		SceneLoader* scene_loader = nullptr;
	private:
		std::function<void(std::unique_ptr<Event>)> event_raiser;
	private:
		std::vector<SceneLayer*> layers;
	public:
		virtual ~Scene() = default;
	public:
		void SetSceneLoader(SceneLoader* loader)
		{
			scene_loader = loader;
		}
		void SetSceneSpriteEngine(ResourceEngine* engine)
		{
			sprite_engine = engine;
		}
		void SetSceneEventRiaser(std::function<void(std::unique_ptr<Event>)> event_raiser)
		{
			this->event_raiser = event_raiser;
		}
	protected:
		unsigned int GetViewPortWidth() const
		{
			return view_port_width;
		}
		unsigned int GetViewPortHeight() const
		{
			return view_port_height;
		}
	public:
		void AddLayer(SceneLayer* layer)
		{
			layers.emplace_back(layer);
		}
		void RemoveLayer(SceneLayer* layer)
		{
			layers.erase(std::find(layers.begin() , layers.end() , layer));
		}
		inline void RegisterEvent(std::unique_ptr<Event> ev)
		{
			event_raiser(std::move(ev));
		}
		inline void SetViewPortSize(unsigned int width, unsigned int height)
		{
			view_port_width = width;
			view_port_height = height;
			RegisterEvent(CreateEventObject(EventResizeScreen{.width = width , .height = height}));
		}
	protected:
		template<typename T , typename... ParamsT>
		void LoadScene(ParamsT&& ... params)
		{
			scene_loader->LoadScene<T>(params...);
		}
	protected:
		Texture CreateTexture(const Image& img)
		{
			return sprite_engine->CreateTexture(img);
		}
		ImageSprite CreateSprite(const Image& img)
		{
			return sprite_engine->CreateSprite(img);
		}
		ImageSprite CreateSprite(Texture texture)
		{
			return sprite_engine->CreateSprite(texture, texture.GetWidth(), texture.GetHeight());
		}
		ImageSprite CreateSprite(Texture texture , unsigned int width , unsigned int height)
		{
			return sprite_engine->CreateSprite(texture, width , height);
		}
		ImageSprite CreateSprite(const Image& img , unsigned int width , unsigned int height)
		{
			return sprite_engine->CreateSprite(CreateTexture(img) , width , height);
		}
	protected:
		ResourceEngine& getCoreEngine()
		{
			return *sprite_engine;
		}
	protected:
		SceneLoader* GetSceneLoader() const
		{
			return scene_loader;
		}
	public:
		virtual void Update() 
		{
			for (auto& layer : layers)
				layer->Update();
		}
		virtual void Initialize()
		{}
		virtual void OnEventReceive(Event& ev)
		{}
	protected:
		DirectX::XMVECTOR GetWindowEdgeDistance(DirectX::XMVECTOR base_position) const
		{
			return DirectX::XMVectorSubtract(DirectX::XMVectorSet(view_port_width, view_port_height, 0, 1) , base_position);
		}
	public:
		void Render(RenderCommandEngine& engine) const
		{
			for (const auto& layer : layers)
			{
				layer->Render(engine);
			}
		}
	};

	class SceneManager : public EngineEventRaiser , public EngineEventSubscriber
	{
	private:
		//SceneLoader* scene_loader;
		ResourceEngine& sprite_engine;
		Scene* current_scene = nullptr;
		std::function<void(Scene*)> scene_deleter;
	public:
		SceneManager(ResourceEngine& engine) : sprite_engine(engine){}
		~SceneManager()
		{
			if (current_scene)
			{
				scene_deleter(current_scene);
			}
		}
	public:
		void OnUpdate() override
		{
			if (current_scene)
			{
				current_scene->Update();
			}
		}
		void OnEventReceive(Event& ev) override
		{
			if (ev.event_type_index == typeid(EventSceneLoaded))
			{
				if(current_scene)
					scene_deleter(current_scene);

				current_scene = GetEventData<EventSceneLoaded>(ev).scene;
				scene_deleter = GetEventData<EventSceneLoaded>(ev).deleter;

				current_scene->SetSceneSpriteEngine(&sprite_engine);
				current_scene->SetSceneEventRiaser([this](std::unique_ptr<Event> ev) 
					{
						RaiseEvent(std::move(ev));
					});
				current_scene->Initialize();
			}
			else if (current_scene)
			{
				current_scene->OnEventReceive(ev);
			}
		}
	};
}