#pragma once
#include <vector>
#include <fstream>
#include <map>
#include "SceneObject.h"
#include "BoxCollider.h"
#include "RepulsiveEngine/ResourceEngine.h"

class Map
{
private:
	std::vector<RipeGrain::SceneObject> map_components;
	std::vector<RipeGrain::BoxCollider> map_colliders;
private:
	std::map<std::string,int> components_with_id;
public:
	Map() = default;
	Map(ResourceEngine& engine , std::filesystem::path mpig_data, std::filesystem::path texture)
	{
		Open(engine, mpig_data, texture);
	}
	void Open(ResourceEngine& engine, std::filesystem::path mpig_data, std::filesystem::path texture)
	{
		map_components.clear();
		map_colliders.clear();

		Texture tex = engine.CreateTexture(texture);
		std::vector<ImageSprite> base_components;

		std::ifstream ifile(mpig_data);
		std::string dta;

		while (ifile.good())
		{
			unsigned int x, y, width, height;
			ifile >> dta;
			if (dta != "----")
			{
				x = std::atoi(dta.c_str());
				ifile >> dta;
				y = std::atoi(dta.c_str());
				ifile >> dta;
				width = std::atoi(dta.c_str());
				ifile >> dta;
				height = std::atoi(dta.c_str());

				auto component = engine.CreateSprite(tex, width + 1, height);
				component.SetTextureCoord(x, y);
				base_components.emplace_back(component);
			}
			else
			{
				break;
			}
		}
		struct ColliderT
		{
			int left, right, top, bottom, index;
		};

		std::vector<ColliderT> colliders_;

		while (ifile.good())
		{
			std::string id;
			int texture_slot, x, y, z, box_top, box_bottom, box_left, box_right;
			ifile >> dta;
			if (dta != "----")
			{
				id = dta;
				ifile >> dta;
				texture_slot = std::atoi(dta.c_str());
				ifile >> dta;
				x = std::atoi(dta.c_str());
				ifile >> dta;
				y = std::atoi(dta.c_str());
				ifile >> dta;
				z = std::atoi(dta.c_str());
				ifile >> dta;
				box_left = std::atoi(dta.c_str());
				ifile >> dta;
				box_top = std::atoi(dta.c_str());
				ifile >> dta;
				box_right = std::atoi(dta.c_str());
				ifile >> dta;
				box_bottom = std::atoi(dta.c_str());

				auto component = base_components[texture_slot];

				RipeGrain::SceneObject obj;
				obj.SetPosition(x, y, z);
				component.SetPosition(DirectX::XMVectorZero());
				obj.AddSprite(component);
				map_components.emplace_back(obj);

				int half_h = component.GetHeight() / 2;
				int half_w = component.GetWidth() / 2;

				/*int top = half_h - box_top;
				int bottom = box_bottom - half_h;
				int left = half_w - box_left;
				int right = box_right - half_w;*/

				bool no_bound = (box_top == 0 && box_bottom == 0 && box_left == 0 && box_right == 0);

				if (!no_bound)
				{
					int top = half_h - box_top;
					int bottom = half_h - (component.GetHeight() - (box_top + box_bottom));
					int left = half_w - box_left;
					int right = half_w - (component.GetWidth() - (box_left + box_right));

					//static_cast<BoxCollider&>(map_components.back()) = BoxCollider{ component.GetPosition() , top , bottom , left , right };

					colliders_.push_back(ColliderT{ .left = left , .right = right ,.top = top , .bottom = bottom , .index = int(map_components.size()) - 1 });
				}

				if (id != "_")
				{
					components_with_id[id] = int(map_components.size() - 1);
				}
			}

			for (auto c : colliders_)
			{
				map_colliders.emplace_back(&map_components[c.index], c.left, c.right, c.top, c.bottom);
			}
		}
	}
public:
	std::vector<RipeGrain::SceneObject>& GetMapComponents()
	{
		return map_components;
	}
	std::vector<RipeGrain::BoxCollider>& GetMapColliders() 
	{
		return map_colliders;
	}
	RipeGrain::SceneObject* GetObjectById(const std::string& id)
	{
		if (auto itr = components_with_id.find(id); itr != components_with_id.end())
			return &map_components[itr->second];
		return nullptr;
	}
};