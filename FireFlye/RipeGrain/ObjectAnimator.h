#pragma once
#include <chrono>
#include <optional>
#include <functional>
#include "BoxCollider.h"
#include "SceneObject.h"
namespace RipeGrain
{
	class ObjectAnimator
	{
	private:
		float per_frame_duration;
	private:
		unsigned int total_frame_count;
	private:
		unsigned int current_frame_index = 0;
	private:
		bool in_reverse = false;
		bool reverse_alternate = false;
	private:
		std::optional<std::chrono::steady_clock::time_point> last_frame_time;
	public:
		ObjectAnimator() = default;
		ObjectAnimator(unsigned int frame_count , float duration, bool reverse_alternate) : per_frame_duration(duration /(float)frame_count) , total_frame_count(frame_count), reverse_alternate(reverse_alternate) {}
	public:
		void SetFrameCount(unsigned int frame_count)
		{
			total_frame_count = frame_count;
		}
		void SetDuration(float duration)
		{
			per_frame_duration = duration / (float)total_frame_count;
		}
		void EnableReversePlay()
		{
			reverse_alternate = true;
		}
		void DisableReversePlay()
		{
			reverse_alternate = false;
		}
	protected:
		virtual void SetObjectFrame(ImageSprite& sprite , unsigned int frame_index) = 0;
	public:
		virtual ~ObjectAnimator() = default;
	public:
		void Animate(ImageSprite& sprite)
		{
			if (!last_frame_time.has_value())
			{
				last_frame_time = std::chrono::steady_clock::now();
				return;
			}

			float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - *last_frame_time).count();
			if (elapsed >= per_frame_duration)
			{
				auto diff = std::clamp(elapsed - per_frame_duration ,0.f , 1.f);
				current_frame_index += reverse_alternate && in_reverse ? -(1) : (1);
				if (current_frame_index >= total_frame_count || current_frame_index < 0)
				{
					SetObjectFrame(sprite, current_frame_index = reverse_alternate && !in_reverse? total_frame_count - 1 : 0);
					last_frame_time = std::nullopt;
					in_reverse = !in_reverse;
					return;
				}
				SetObjectFrame( sprite, current_frame_index);
				last_frame_time = std::chrono::steady_clock::now();
			}
		}
	public:
		void Stop()
		{
			last_frame_time = std::nullopt;
		}
	public:
		bool IsStopped()
		{
			return !(last_frame_time.has_value());
		}
	};

	class SpriteSheetAnimator : public ObjectAnimator
	{
	private:
		std::vector<std::pair<unsigned int, unsigned int>> sheet_offsets;
	public:
		SpriteSheetAnimator(std::vector<std::pair<unsigned int, unsigned int>> sheet_offsets , float duration_ms) 
			:
		 ObjectAnimator(sheet_offsets.size() , duration_ms, false) , sheet_offsets(sheet_offsets)
		{}
	protected:
		void SetObjectFrame(ImageSprite& sprite , unsigned int frame_index) override
		{
			auto [x, y] = sheet_offsets[frame_index];
			sprite.SetTextureCoord(x, y);
		}
	};

	class PredSpriteSheetAnimator : public ObjectAnimator
	{
	private:
		std::function<std::pair<unsigned int, unsigned int>(int)> offset_pred;
	protected:
		void SetObjectFrame(ImageSprite& sprite, unsigned int frame_index) override
		{
			auto [x, y] = offset_pred(frame_index);
			sprite.SetTextureCoord(x, y);
		}
	public:
		void SetPredicator(decltype(offset_pred) pred)
		{
			offset_pred = pred;
		}
	};

	class TextureBatchAnimator : public ObjectAnimator
	{
	private:
		std::vector<Texture> textures;
	public:
		TextureBatchAnimator(std::vector<Texture> textures, float duration_ms, bool reverse_alternate = false)
			:
			ObjectAnimator(textures.size() , duration_ms , reverse_alternate) , textures(textures)
		{}
	protected:
		void SetObjectFrame(ImageSprite& sprite , unsigned int frame_index) override
		{
			sprite.SetTexture(textures[frame_index]);
		}
	};

	class PositionAnimator : public ObjectAnimator
	{
	private:
		int velocity;
		int start_x;
		int end_x;
	public:
		PositionAnimator(int start_x, int end_x, float duration_ms) 
			:
		 ObjectAnimator(std::abs(end_x - start_x) , duration_ms , false) , start_x(start_x) , end_x(end_x)
		{
			velocity = start_x < end_x ? 1 : -1;
		}
	protected:
		void SetObjectFrame(ImageSprite& sprite , unsigned int index) override
		{
			int new_x = start_x + index * velocity;
			int old_y = DirectX::XMVectorGetY(sprite.GetPosition());
			int old_z = DirectX::XMVectorGetZ(sprite.GetPosition());
			sprite.SetPosition(DirectX::XMVectorSet(new_x, old_y, old_z , 1));
		}
	};

	class MovementAnimator
	{
	private:
		float t = 1.0f;
		float speed = 0.05;
		DirectX::XMVECTOR strt;
		DirectX::XMVECTOR dest;
	public:
		void Cancel()
		{
			t = 1;
			strt = dest = DirectX::XMVectorZero();
		}
		void Cancel(RipeGrain::SceneObject* object, const RipeGrain::BoxCollider& c1, const RipeGrain::BoxCollider& c2)
		{
			while (c1.IsCollidingWith(c2))
			{
				object->SetPosition(DirectX::XMVectorLerp(strt, dest, t -= 0.02));
			}
			dest = object->GetPosition();
			t = 1.0f;
		}
		void SetDestination(DirectX::XMVECTOR pos_start, DirectX::XMVECTOR pos_end, float speed = 0.2)
		{
			t = 0.0f;
			this->speed = speed;
			strt = pos_start;
			dest = pos_end;
		}
		void Update(RipeGrain::SceneObject* obj)
		{
			if (t >= 1.0f)
				return;
			t += speed;
			obj->SetPosition(DirectX::XMVectorLerp(strt, dest, t));
		}
		bool ReachedDestination() const
		{
			return t >= 1.0f;
		}
	public:
		std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR> GetDestination() const
		{
			return { strt , dest };
		}
	};
}