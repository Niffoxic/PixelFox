#pragma once

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/window_manager/inputs/keyboard_inputs.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "ai/projectile/straight/straight_projectile.h"

//~ states
#include "player_state/player_state.h"


namespace pixel_game
{
	class PlayerCharacter
	{
	public:
		PlayerCharacter () = default;
		~PlayerCharacter() = default;

		PlayerCharacter(const PlayerCharacter&) = delete;
		PlayerCharacter(PlayerCharacter&&) = delete;

		PlayerCharacter& operator=(const PlayerCharacter&) = delete;
		PlayerCharacter& operator=(PlayerCharacter&&) = delete;

		bool Initialize();
		void Update    (float deltaTime);
		void Release   ();

		pixel_engine::PEISprite*       GetPlayerBody() const;
		pixel_engine::AnimSateMachine* GetPlayerAnimState() const;

		//~ Handle Inputs
		void HandleInput(
			pixel_engine::PEKeyboardInputs* keyboard,
			float deltaTime);

		void Draw();
		void Hide();
		void UnloadFromQueue();

		bool IsInitialized() const { return m_bInitialized; }

		//~ states
		void  SetHealth	    (float hp) {}
		float GetPlayerHeath() const { return 100.f; }
		
		void SetDashAnimDone(bool v) noexcept { m_dashAnimDone = v; }
		bool IsDashAnimDone() const noexcept { return m_dashAnimDone; }

		uint64_t BumpDashSerial() noexcept { return ++m_dashAnimSerial; }
		uint64_t CurrentDashSerial() const noexcept { return m_dashAnimSerial; }

		void SetNearestTargetLocation(const FVector2D& pos) { m_nearestLoc = pos; }
		void SetDenseTargetLocation(const FVector2D& pos)   { m_bestLoc = pos; }

	private:
		//~ Look
		bool InitializePlayer();
		bool InitializeAppearance();
		void UpdatePlayerAppearance(float deltaTime);

		//~ anim state
		void UpdatePlayerState();

		//~ Actions
		void Attack(float deltaTime);

		//~ Callbacks

		//~ Events 

		//~ helpers
		_NODISCARD _Check_return_ _Success_(return != nullptr)
		inline bool ValidatePathExists(const std::string & path) const
		{
			if (!std::filesystem::exists(path))
			{
				pixel_engine::logger::error("path dNT: {}", path);
				return false;
			}
			return true;
		}

	private:
		std::unique_ptr<StraightProjectile> m_pBasicAttack{ nullptr };
		std::unique_ptr<StraightProjectile> m_pSpecialAttack{ nullptr };

		FVector2D m_nearestLoc{ 10000.f, 10000.f};
		FVector2D m_bestLoc{ 10000.f, 10000.f};
		
		float     m_nAttackDistance	   { 10.f };
		float     m_nFireCoolDown	   { 0.2f };
		FVector2D m_MuzzleOffset	   { 0.5f, 0.f };
		float     m_nProjectileSpeed   { 15.f };
		float     m_nProjectileLifeSpan{ 0.6f };
		float     m_nProjectileDamage  { 20.f };

		pixel_engine::PEKeyboardInputs* m_pKeyboard;
		std::string m_szPlayerBase{"assets/player/"};
		bool m_bInitialized{ false };
		std::unique_ptr<pixel_engine::QuadObject>       m_pBody    { nullptr };
		std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState{ nullptr };

		struct PlayerParams
		{
			float movementSpeed	   { 5.f };
			float dashCooldownTimer{ 0.0f };
			float dashCooldown	   { 1.5f };
			float dashForce        { 10.f };
		} m_playerState;

		//~ states management
		EPlayerStateId m_eState{ EPlayerStateId::Idle };
		IPlayerState* m_pState{ nullptr };

		// input and cache
		FVector2D m_direction{ 0.f, 0.f };
		FVector2D m_lastNonZeroDir{ 1.f, 0.f };
		float m_cachedDt{ 0.f };

		float m_dashDuration{ 0.12f };
		float m_dashTimer{ 0.f };
		bool     m_dashAnimDone = false;
		uint64_t m_dashAnimSerial = 0;

		float m_specialCooldown{ 0.f };
		float m_specialCooldownTimer{ 0.f };
	};
} // pixel_game
