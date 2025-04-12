#pragma once
#include <vector>
#include "RepulsiveEngine/ImageSprite.h"
namespace RipeGrain
{
	class SceneObject
	{
	protected:
		DirectX::XMVECTOR position;
		DirectX::XMMATRIX transformation;
	protected:
		std::vector<ImageSprite> sprites;
	public:
		SceneObject() : position(DirectX::XMVectorZero()) , transformation(DirectX::XMMatrixIdentity()){}
		virtual ~SceneObject() = default;
	public:
		inline void AddSprite(ImageSprite sprite)
		{
			sprites.emplace_back(sprite);
		}
	public:
		std::vector<ImageSprite>& GetSprites()
		{
			return sprites;
		}
	public:
		virtual void Update() {}
	public:
		inline void SetX(int x)
		{
			position = DirectX::XMVectorSetX(position, x);
		}
		inline void SetY(int y)
		{
			position = DirectX::XMVectorSetY(position, y);
		}
		inline void SetZ(int z)
		{
			position = DirectX::XMVectorSetZ(position, z);
		}
		inline void SetPosition(int x, int y)
		{
			SetPosition(x, y, GetZ());
		}
		inline void SetPosition(int x , int y , int z)
		{
			position = DirectX::XMVectorSet(x, y, z, 1);
		}
		inline void SetPosition(DirectX::XMVECTOR position)
		{
			this->position = position;
		}
	public:
		inline void SetScaling(float x, float y, float z)
		{
			transformation = DirectX::XMMatrixScaling(x, y, z);
		}
		inline void SetTransformation(DirectX::XMMATRIX transformation)
		{
			this->transformation = transformation;
		}
		inline DirectX::XMMATRIX GetTransformation() const
		{
			return transformation;
		}
	public:
		inline int GetX() const
		{
			return DirectX::XMVectorGetX(position);
		}
		inline int GetY() const
		{
			return DirectX::XMVectorGetY(position);
		}
		inline int GetZ() const
		{
			return DirectX::XMVectorGetZ(position);
		}
		inline std::pair<int , int> GetXY() const
		{
			return { GetX() , GetY() };
		}
		inline DirectX::XMVECTOR GetPosition() const
		{
			return position;
		}
	};
}