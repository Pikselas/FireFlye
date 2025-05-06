#pragma once
#include <typeindex>
#include <optional>
#include <functional>
#include "SceneObject.h"
#include "BoxCollider.h"

namespace RipeGrain
{
	struct EventResizeScreen
	{
		unsigned int width;
		unsigned int height;
	};

	struct EventCollidersAdded
	{
		std::vector<BoxCollider> colliders;
	};

	struct EventCollisionDetected
	{
		std::vector<std::pair<BoxCollider, std::vector<BoxCollider>>>& collision_list;
	};

	struct EventSceneLoaded
	{
		class Scene* scene;
		std::function<void(Scene*)> deleter = nullptr;
	};

	struct EventMouseInput
	{
		enum class Type
		{
			Move,
			Wheel,
			LeftPress, 
			LeftRelease,
			RightPress,
			RightRelease,
			LeftDoublePress,
			RightDoublePress,
		};
		Type type;
		int delta;
		int x_pos, y_pos;
		int client_x, client_y;
	};

	struct EventKeyBoardInput
	{
		enum class Type
		{
			KeyPress,
			CharInput,
			KeyRelease
		};
		Type type;
		bool is_repeated;
		unsigned char key_code;
	};

	class Event
	{
		enum class Type
		{
			CustomEvent
		};
	public:
		Type event_type = Type::CustomEvent;
		std::type_index event_type_index;
		class EngineComponent* sender = nullptr;
	public:
		Event(std::type_index type) : event_type_index(type){}
	};

	template<typename EventType>
	class EventObject : public Event
	{
	public:
		EventType data;
	public:
		EventObject(EventType data) : Event(typeid(EventType)) , data(data) {}
	};

	template<typename T>
	std::unique_ptr<Event> CreateEventObject(T data)
	{
		return std::make_unique<EventObject<T>>(std::move(data));
	}

	template<typename T> 
	T& GetEventData(Event& event_data)
	{
		return reinterpret_cast<EventObject<T>&>(event_data).data;
	}
}