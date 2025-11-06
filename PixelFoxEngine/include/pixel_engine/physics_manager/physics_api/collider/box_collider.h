#pragma once

#include "PixelFoxEngineAPI.h"

#include "fox_math/vector.h"
#include "fox_math/transform.h"
#include "core/unordered_map.h"
#include "core/vector.h"
#include "pixel_engine/physics_manager/physics_api/rigid_body/rigid_body.h"

#include <functional>
#include <cstdint>
#include <string>
#include <cmath>

namespace pixel_engine
{
	struct Contact;

	enum class ColliderType : uint8_t
	{
		Dynamic,
		Static,
		Trigger
	};

	class BoxCollider;
	typedef struct _ON_HIT_CALLBACK
	{
		BoxCollider* target{ nullptr };
		std::string Tag    { "NoTag" };
		std::function<void()> m_fnOnTriggerEnter;
		std::function<void()> m_fnOnTriggerExit;
	} ON_HIT_CALLBACK;

	class PFE_API BoxCollider
	{
	public:
		BoxCollider(RigidBody2D* rigidBody);
		~BoxCollider() = default;

		BoxCollider(const BoxCollider&) = default;
		BoxCollider(BoxCollider&&) = default;
		BoxCollider& operator=(const BoxCollider&) = default;
		BoxCollider& operator=(BoxCollider&&) = default;

		//~ Update physics transform
		void Update(float deltaTime);

		//~ Collision handling
		bool CheckCollision(BoxCollider* other, Contact& outContact);  // AABB/OBB collision test
		void RegisterCollision(BoxCollider* other, const Contact& contact);

		//~ Callbacks
		void SetOnHitEnterCallback(std::function<void(BoxCollider*)>&& callback);
		void SetOnHitExitCallback(std::function<void(BoxCollider*)>&& callback);
		void AddCallback(const ON_HIT_CALLBACK& callback);

		//~ Collider configuration
		void SetScale(const FVector2D& scale);
		FVector2D GetScale() const;

		void SetOffset(const FVector2D& offset);
		FVector2D GetOffset() const;

		void SetColliderType(ColliderType type);
		ColliderType GetColliderType() const;

		//~ Utility getters
		FTransform2D GetTransform2D() const;
		RigidBody2D* GetRigidBody2D() const;

		FVector2D GetMin() const; 
		FVector2D GetMax() const;
		FVector2D GetWorldCenter() const;
		FVector2D GetHalfExtents() const;     
		FVector2D GetLastContactNormal() const;

		//~ Collider state
		bool IsTrigger() const;
		bool IsDynamic() const;
		bool IsStatic() const;

		//~ tag
		bool HasTag   (const std::string& tag);
		bool AttachTag(const std::string& tag);
		void DetachTag(const std::string& tag);

	private:
		//~ helpers
		float AbsF(float v) const { return v < 0.f ? -v : v; }
		FVector2D AbsV(const FVector2D& v) const { return { AbsF(v.x), AbsF(v.y) }; }

	private:
		//~ game tag
		fox::unordered_map<std::string, bool> m_tags;

		//~ Internal data
		FVector2D    m_scale		{ 1.0f, 1.0f };       
		FVector2D    m_offset		{ 0.0f, 0.0f };
		ColliderType m_colliderType	{ ColliderType::Static };
		RigidBody2D* m_pRigidBody	{ nullptr };

		FVector2D m_lastContactNormal{ 0.f, 0.f };

		struct CollisionCallBack
		{
			bool ColliderEntered{ false };
			bool ColliderExited { false };
			std::function<void()> fnOnTriggerEnter;
			std::function<void()> fnOnTriggerExit;
		};
		fox::unordered_map<BoxCollider*, CollisionCallBack> m_collidersTrack{};
		
		//~ fires each time
		fox::unordered_map<BoxCollider*, bool> m_hitTrack{};
		std::function<void(BoxCollider*)> m_fnOnHitEnterCallback;
		std::function<void(BoxCollider*)> m_fnOnHitExitCallback;
	};
} // namespace fox_physics
