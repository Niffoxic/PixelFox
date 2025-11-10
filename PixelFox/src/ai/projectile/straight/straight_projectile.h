#pragma once

#include "ai/projectile/interface_projectile.h"
#include "pixel_engine/physics_manager/physics_queue.h"
#include <sal.h>
#include <cfloat>

namespace pixel_game
{
	class StraightProjectile final : public IProjectile
	{
	public:
		using OnHitCB = std::function<void(_In_ IProjectile* projectile, _In_ pixel_engine::BoxCollider*)>;

	public:
		StraightProjectile() = default;
		~StraightProjectile() override = default;

		//~ Lifecycle
		_NODISCARD _Check_return_
		bool Init(INIT_PROJECTILE_DESC& desc) override;

		void Update(_In_ float deltaTime) override;

		_NODISCARD _Check_return_
		bool Release() override;

		//~ Firing
		_NODISCARD _Check_return_
		bool Fire(
			_In_ const FVector2D& worldPos,
			_In_ const FVector2D& directionNorm,
			_In_ float speed = 0.0f) override;

		//~ State control
		void Deactivate() override;
		void SetActive(_In_ bool on) override;

		_NODISCARD _Check_return_
		bool IsActive() const override;

		//~ Accessors
		_NODISCARD _Check_return_ _Ret_maybenull_
		pixel_engine::PEISprite*   GetBody    () const override;

		_NODISCARD _Check_return_ _Ret_maybenull_
		pixel_engine::BoxCollider* GetCollider() const override;

		//~ Transform
		void SetPosition(_In_ const FVector2D& pos) override;

		_NODISCARD _Check_return_
		FVector2D GetPosition() const override;

		void SetDirection(_In_ const FVector2D& dirNorm) override;

		_NODISCARD _Check_return_
		FVector2D GetDirection() const override;

		//~ Motion
		void SetSpeed(_In_ float unitsPerSecond) override;

		_NODISCARD _Check_return_
		float GetSpeed() const override;

		//~ Lifetime
		_NODISCARD _Check_return_
		bool SetLifeSpan(_In_ float seconds) override;

		_NODISCARD _Check_return_
		float GetLifeSpan() const override;

		_NODISCARD _Check_return_
		float GetTimeLeft() const override;

		//~ Damage
		void SetDamage(_In_ float amount) override;

		_NODISCARD _Check_return_
		float GetDamage() const override;

		//~ Collision
		void OnHit(pixel_engine::BoxCollider* collider) override;

		void AddHitTag(const std::string& tag) override;
		void RemoveHitTag(const std::string& tag) override;
		bool HasHitTag(const std::string& tag) const override;

		pixel_engine::AnimSateMachine* GetAnimStateMachine() const override;

	private:
		//~ helper
		inline float AngleFromDir(const FVector2D& d) noexcept
		{
			return std::atan2(d.y, d.x);
		}

		void SetCallback();

	private:
		fox::vector<std::string> m_ppszTags{};
		pixel_engine::PEISprite* m_pOwner{ nullptr };
		std::unique_ptr<pixel_engine::QuadObject> m_pBody{ nullptr };

		bool      m_bActive   { false };
		float     m_nSpeed    { 12.0f };
		float     m_nLifeSpan { 2.0f };
		float     m_nTimeLeft { 4.0f };
		float     m_nDamage   { 10.0f };
		FVector2D m_direction { 0.f, 0.f };

		//~ Callback
		std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState;
		std::function<void()> m_fnOnFire;
		OnHitCB m_fnOnHit;
		std::function<void()> m_fnOnExpired;
		std::function<void()> m_fnOnActive;
	};
} // namespace pixel_game
