#pragma once
#include "SceneObject.h"
namespace RipeGrain
{
	class BoxCollider
	{
	private:
		int top;
		int left;
		int right;
		int bottom;
	private:
		SceneObject* object;
	public:
		BoxCollider() : BoxCollider(0 , 0 , 0 , 0 ){}
		BoxCollider(int left, int right, int top, int bottom) : BoxCollider(nullptr, left, right, top, bottom) {}
		BoxCollider(SceneObject* obj, int left, int right, int top, int bottom ) : object(obj), left(left), right(right), top(top), bottom(bottom) {}
	public:
		bool operator==(const BoxCollider& other) const
		{
			return  object == other.object && object != nullptr ? true : left == other.left && right == other.right && top == other.top && bottom == other.bottom;
		}
	public:
		inline int GetCenterX() const
		{
			return (GetRight() + GetLeft()) / 2;
		}
		inline int GetCenterY() const
		{
			return (GetBottom() + GetTop()) / 2;
		}
	public:
		inline int GetLeft() const
		{
			if(object)
				return object->GetX() - left;
			return left;
		}
		inline int GetRight() const
		{
			if (object)
				return object->GetX() + right;
			return right;
		}
		inline int GetTop() const
		{
			if (object)
				return object->GetY() - top;
			return top;
		}
		inline int GetBottom() const
		{
			if (object)
				return object->GetY() + bottom;
			return bottom;
		}
	public:
		inline DirectX::XMVECTOR GetPosition() const
		{
			if(object)
				return object->GetPosition();
			return DirectX::XMVectorSet(left + (right - left) / 2, top + (bottom - top) / 2, 1, 1);
		}
	public:
		bool IsCollidingWith(BoxCollider other) const
		{
			return GetLeft() <= other.GetRight() && GetRight() >= other.GetLeft() && GetTop() <= other.GetBottom() && GetBottom() >= other.GetTop();
		}
	public:
		BoxCollider GetStaticCollider() const
		{
			return BoxCollider{GetLeft() , GetRight() , GetTop() , GetBottom()};
		}
		int GetArea() const
		{
			return (GetRight() - GetLeft()) * (GetBottom() - GetTop());
		}
		SceneObject* GetInlineObject() const
		{
			return object;
		}
	};
}