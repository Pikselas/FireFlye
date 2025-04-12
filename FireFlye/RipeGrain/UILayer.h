#pragma once
#include <queue>
#include "SceneLayer.h"
#include "UIComponent.h"
#include "RepulsiveEngine/StencilBuffer.h"

namespace RipeGrain
{
	class UILayer final : public SceneLayer
	{
	private:
		std::list<UIComponent> components;
		std::queue<std::pair<UIComponent::UIPtr , bool>> removed_components;
	public:
		mutable StencilBuffer ui_clip_buffer;
	public:
		UIComponent::UIPtr AddUIFrame(UIComponent component)
		{
			auto& frame = components.emplace_back(component);
			frame.remove_self = [this](UIComponent::UIPtr ptr)
				{
					removed_components.emplace(ptr, false);
				};
			return frame.self = std::prev(components.end());
		}
		UIComponent::UIPtr AddUIFrame(UIComponentDescription desc)
		{
			return AddUIFrame(UIComponent::create_component(desc));
		}
		void RemoveUIFrame(UIComponent::UIPtr frame)
		{
			removed_components.emplace(frame, true);
		}
	public:
		std::list<UIComponent>& getComponents()
		{
			return components;
		}
	public:
		void Render(RenderCommandEngine& engine) const override
		{
			engine.SetStencilBuffer(ui_clip_buffer);
			for (const auto& ui_component : components)
			{
				engine.ClearStencilBuffer(ui_clip_buffer);
				ui_component.Render(engine, DirectX::XMVectorGetX(base_position), DirectX::XMVectorGetY(base_position) , 0);
			}
			engine.EndStencilClipping(1);
			engine.RemoveStencilBuffer();
		}
		void Update() override
		{
			while (!removed_components.empty())
			{
				auto [ptr, is_base_component] = removed_components.front();
				if (is_base_component)
				{
					components.erase(ptr);
				}
				else if(ptr->parent)
				{
					(*ptr->parent)->children.erase(ptr);
				}
				removed_components.pop();
			}
		}
	};

	struct EventRegisterUI
	{
		UILayer* layer = nullptr;
	};

	std::unique_ptr<Event> CreateUIRegisterEvent(UILayer* layer)
	{
		return CreateEventObject<EventRegisterUI>(EventRegisterUI{ layer });
	}
}