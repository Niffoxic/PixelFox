// player_state_dash.h
#pragma once
#include "player/player_state/player_state.h"
#include "world/state/character_state.h"

namespace pixel_game
{
    class PlayerStateDash final : public PlayerStateBase
    {
    public:
        EPlayerStateId   Id()   const noexcept override { return EPlayerStateId::Dash; }
        std::string_view Name() const noexcept override { return "Dash"; }

        bool IsOneShot() const noexcept override { return true; }
        EInterruptPolicy InterruptPolicy() const noexcept override { return EInterruptPolicy::Uninterruptible; }
        bool ControlsMovementDirectly() const noexcept override { return true; }

        void OnEnter(PlayerContext& ctx) override;
        void OnExit(PlayerContext& ctx) override;

        EPlayerStateId Tick(PlayerContext& ctx) override;
    };
}
