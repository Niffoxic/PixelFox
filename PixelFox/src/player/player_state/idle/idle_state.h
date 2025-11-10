// player_state_idle.h
#pragma once
#include "player/player_state/player_state.h"
#include "world/state/character_state.h"

namespace pixel_game
{
    class PlayerStateIdle final : public PlayerStateBase
    {
    public:
        EPlayerStateId Id()   const noexcept override { return EPlayerStateId::Idle; }
        std::string_view Name() const noexcept override { return "Idle"; }

        void OnEnter(PlayerContext& ctx) override;
        EPlayerStateId Tick(PlayerContext& ctx) override;
    };
}
