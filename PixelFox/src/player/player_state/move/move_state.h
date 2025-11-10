// player_state_move.h
#pragma once
#include "player/player_state/player_state.h"
#include "world/state/character_state.h"

namespace pixel_game
{
    class PlayerStateMove final : public PlayerStateBase
    {
    public:
        EPlayerStateId   Id()   const noexcept override { return EPlayerStateId::Move; }
        std::string_view Name() const noexcept override { return "Move"; }

        void OnEnter(PlayerContext& ctx) override;

        EPlayerStateId Tick(PlayerContext& ctx) override;
    };
}
