#pragma once

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/window_manager/inputs/keyboard_inputs.h"
#include "pixel_engine/utilities/logger/logger.h"

namespace pixel_game
{
	class PlayerCharacter
	{
	public:
		PlayerCharacter() = default;
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
			const pixel_engine::PEKeyboardInputs* keyboard,
			float deltaTime);

		void Draw();
		void Hide();
		void UnloadFromQueue();

		bool IsInitialized() const { return m_bInitialized; }

	private:
		//~ Look
		bool InitializePlayer();
		bool InitializeAppearance();
		void UpdatePlayerAppearance(float deltaTime);

		//~ Actions

		//~ Callbacks

		//~ Events 

	private:
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

		__forceinline _NODISCARD _Check_return_
		int DirToOctant(_In_ const FVector2D& dir) const noexcept
		{
			float eps = 1e-6f; //~ fixes wierd bug where even tho it should be zero!
			if (std::fabs(dir.x) < eps && std::fabs(dir.y) < eps)
			{
				return -1;
			}

			float ang = std::atan2(-dir.y, dir.x);
			float pi = fox_math::PI_v<float>;
			float oct = pi / 4.0f;

			if (ang < 0.0f) ang += 2.0f * pi;

			return static_cast<int>(std::floor((ang + oct * 0.5f) / oct)) & 7;
		}

	private:
		std::string m_szPlayerBase{};
		bool m_bInitialized{ false };
		std::unique_ptr<pixel_engine::QuadObject>       m_pBody    { nullptr };
		std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState{ nullptr };

		struct PlayerState
		{
			float movementSpeed	   { 5.f };
			float dashCooldownTimer{ 0.f };
			float dashCooldown	   { 1.5f };
			float dashForce        { 8.f };
		} m_playerState;
	};
} // pixel_game
