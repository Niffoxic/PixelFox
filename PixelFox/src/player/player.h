#pragma once

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/window_manager/inputs/keyboard_inputs.h"

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
