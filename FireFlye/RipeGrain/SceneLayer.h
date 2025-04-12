#pragma once
#include "SceneObject.h"
namespace RipeGrain
{
	class SceneLayer
	{
	protected:
		DirectX::XMVECTOR base_position = DirectX::XMVectorZero();
	public:
		inline void SetBasePosition(int x, int y)
		{
			base_position = DirectX::XMVectorSet(x, y, 0, 1);
		}
		inline void SetBasePosition(DirectX::XMVECTOR pos)
		{
			base_position = pos;
		}
		inline const DirectX::XMVECTOR& GetBasePosition() const
		{
			return base_position;
		}
		inline void Scroll(DirectX::XMVECTOR dir, DirectX::XMVECTOR max_size)
		{
			auto pos = DirectX::XMVectorAdd(base_position, dir);
			pos = DirectX::XMVectorClamp(pos, DirectX::XMVectorNegate(max_size), DirectX::XMVectorZero());
			base_position = pos;
		}
	public:
		virtual void Update() {};
		virtual void Render(RenderCommandEngine&) const {};
	};

	class ObjectsLayer final : public SceneLayer
	{
	protected:
		std::vector<SceneObject*> objects;
	public:
		void AddObject(SceneObject* obj)
		{
			objects.emplace_back(obj);
		}
		void RemoveObject(SceneObject* obj)
		{
			objects.erase(std::find(objects.begin(), objects.end(), obj));
		}
	private:
		static bool object_sort_predicator(SceneObject* obj1, SceneObject* obj2)
		{
			return DirectX::XMVectorGetZ(obj1->GetPosition()) < DirectX::XMVectorGetZ(obj2->GetPosition());
		}
	public:
		inline void sortSceneObjectsByZ()
		{
			std::sort(objects.begin(), objects.end(), object_sort_predicator);
		}
	public:
		virtual void Update() override
		{
			for (auto obj : objects)
			{
				obj->Update();
			}
		}
	public:
		void Render(RenderCommandEngine& engine) const override
		{
			for (const auto& obj : objects)
			{
				const auto transform = obj->GetTransformation();
				for (auto sprite : obj->GetSprites())
				{
					auto pos = DirectX::XMVectorAdd(obj->GetPosition(), sprite.GetPosition());
					pos = DirectX::XMVectorAdd(base_position, pos);
					sprite.SetPosition(pos);
					auto transform2 = sprite.GetTransformation() * transform;
					sprite.SetTransformation(transform2);
					sprite.Draw(engine);
				}
			}
		}
	};
}