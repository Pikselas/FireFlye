#pragma once
#include <list>
#include <functional>
#include "Event.h"
#include "RepulsiveEngine/ImageSprite.h"
#include "RepulsiveEngine/StencilBuffer.h"

namespace RipeGrain
{
	struct UIComponentDescription
	{
		int position_x;
		int position_y;
		ImageSprite ui_sprite;
	};

	class UIComponent
	{
		friend class UILayer;
	private:
		ImageSprite ui_sprite;
	public:
		bool Hidden = false;
	    bool Focused = false;
	private:
		unsigned int max_page_size = 0;
	private:
		DirectX::XMVECTOR page_position = DirectX::XMVectorZero();
	public:
		using UIPtr = std::list<UIComponent>::iterator;
	private:
		UIPtr self;
		std::optional<UIPtr> parent;
		std::function<void(UIPtr)> remove_self;
	public:
		std::function<void(EventMouseInput)> on_mouse = nullptr;
		std::function<void(EventKeyBoardInput)> on_keyboard = nullptr;
	private:
		std::list<UIComponent> children;
	public:
		bool IsInRange(int x_pos, int y_pos)
		{
			int ui_x = GetX();
			int ui_y = GetY();

			int width = ui_sprite.GetWidth();
			int height = ui_sprite.GetHeight();

			return (x_pos >= ui_x && x_pos <= ui_x + width && y_pos >= ui_y && y_pos <= ui_y + height);
		}
		void OnEvent(EventMouseInput evnt)
		{
			if (Hidden)
				return;

			if(on_mouse)
				on_mouse(evnt);

			Focused = true;

			evnt.x_pos -= DirectX::XMVectorGetX(page_position);
			evnt.y_pos -= DirectX::XMVectorGetY(page_position);

			for (auto& child : children)
			{
				if (child.IsInRange(evnt.x_pos , evnt.y_pos))
				{
					evnt.x_pos = evnt.x_pos - child.GetX();
					evnt.y_pos = evnt.y_pos - child.GetY();
					child.OnEvent(evnt);
				}
				else
				{
					child.Focused = false;
				}
			}
		}
		void OnEvent(EventKeyBoardInput evnt)
		{
			if (on_keyboard)
				on_keyboard(evnt);

			for (auto& child : children)
			{
				if (child.Focused)
				{
					child.OnEvent(evnt);
				}
			}
		}
	public:
		UIComponent() = default;
		UIComponent(ImageSprite ui_sprite) : ui_sprite(ui_sprite) , max_page_size(ui_sprite.GetHeight()){}
	private:
		static UIComponent create_component(UIComponentDescription desc)
		{
			UIComponent comp(desc.ui_sprite);
			comp.SetPosition(desc.position_x, desc.position_y);
			return comp;
		}
	public:
		unsigned int GetHeight() const
		{
			return ui_sprite.GetHeight();
		}
		unsigned int GetWidth() const
		{
			return ui_sprite.GetWidth();
		}
		int GetX() const
		{
			return DirectX::XMVectorGetX(ui_sprite.GetPosition());
		}
		int GetY() const
		{
			return DirectX::XMVectorGetY(ui_sprite.GetPosition());
		}
	public:
		void SetX(int x)
		{
			ui_sprite.SetPosition(DirectX::XMVectorSetX(ui_sprite.GetPosition(), x));
		}
		void SetY(int y)
		{
			ui_sprite.SetPosition(DirectX::XMVectorSetY(ui_sprite.GetPosition(), y));
		}
		void SetPosition(int x, int y)
		{
			SetX(x);
			SetY(y);
		}
		void SetPosition(DirectX::XMVECTOR pos)
		{
			ui_sprite.SetPosition(pos);
		}
	public:
		UIPtr AddComponent(UIComponent component)
		{
			max_page_size = (std::max)(max_page_size, component.GetHeight() + component.GetY());
			auto& child = children.emplace_back(std::move(component));
			child.parent = self;
			child.remove_self = remove_self;
			return child.self = std::prev(children.end());
		}
		UIPtr AddComponent(UIComponentDescription desc)
		{
			return AddComponent(create_component(desc));
		}
	public:
		void SetUITexture(Texture texture)
		{
			ui_sprite.SetTexture(texture);
		}
	public:
		void ScrollBy(int y)
		{
			auto pos = DirectX::XMVectorAdd(page_position, DirectX::XMVectorSet(0, y, 0, 0));
			auto min = DirectX::XMVectorSet(0, (int)ui_sprite.GetHeight() - (int)max_page_size, 0, 0);
			page_position = DirectX::XMVectorClamp(pos , min , DirectX::XMVectorZero());
		}
	public:
		void Remove()
		{
			remove_self(self);
		}
		void RemoveComponent(UIPtr child)
		{
			child->Remove();
		}
		void Render(RenderCommandEngine& engine, int parent_x, int parent_y , unsigned int stencil_ref) const
		{
			if (Hidden)
				return;

			auto sprite = ui_sprite;
			auto half_size = DirectX::XMVectorDivide(DirectX::XMVectorSet(sprite.GetWidth(), sprite.GetHeight(), 1, 1), DirectX::XMVectorSet(2, 2, 1, 1));
			auto pos = DirectX::XMVectorAdd(sprite.GetPosition(), half_size);

			pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorSet(parent_x, parent_y, 1, 1));
			sprite.SetPosition(pos);
			sprite.Draw(engine);
			auto child_pos = DirectX::XMVectorAdd(ui_sprite.GetPosition(), page_position);
			child_pos = DirectX::XMVectorAdd(child_pos , DirectX::XMVectorSet(parent_x, parent_y, 1, 1));
			for (const auto& child_ui : children)
			{
				engine.BeginStencilClipping(stencil_ref + 1);
				child_ui.Render(engine, DirectX::XMVectorGetX(child_pos), DirectX::XMVectorGetY(child_pos),stencil_ref + 1);
				engine.EndStencilClipping(stencil_ref + 1);
			}
		}
	public:
		void Clear()
		{
			page_position = DirectX::XMVectorZero();
			max_page_size = 0;
			children.clear();
		}
	};

	inline void SetTextAtMiddle(Image& img, ColorType color ,  std::wstring s, Font& font)
	{
		auto [txt_w, txt_h] = font.CalculateTextSize(s);
		txt_h /= 2;
		txt_w /= 2;
		auto y = img.GetHeight() / 2 - txt_h;
		auto x = img.GetWidth() / 2 - txt_w;

		img.DrawString(s,color ,x , y , font);
	}
	inline void SetTextMiddleHorizontally(Image& img, ColorType color, std::wstring s, Font& font , unsigned int y_pos)
	{
		auto [txt_w, txt_h] = font.CalculateTextSize(s);
		txt_w /= 2;
		auto x = img.GetWidth() / 2 - txt_w;
		img.DrawString(s, color, x, y_pos, font);
	}
}