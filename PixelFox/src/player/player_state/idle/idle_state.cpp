#include "idle_state.h"

using namespace pixel_game;

void PlayerStateIdle::OnEnter(PlayerContext& ctx)
{
    const FVector2D d = ResolveFacing(ctx);
    const int oct = ToOctant(ctx, d);

    switch (oct)
    {
    case 0: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_RIGHT)); break;
    case 1: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_UP_RIGHT)); break;
    case 2: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_UP)); break;
    case 3: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_UP_LEFT)); break;
    case 4: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_LEFT)); break;
    case 5: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_DOWN_LEFT)); break;
    case 6: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_DOWN)); break;
    case 7: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_DOWN_RIGHT)); break;
    default: ctx.anim->TransitionTo(ToString(CharacterState::IDLE_RIGHT)); break;
    }
}

EPlayerStateId PlayerStateIdle::Tick(PlayerContext& ctx)
{
    if (ctx.dir.x != 0.f || ctx.dir.y != 0.f)
        return EPlayerStateId::Move;

    if (ctx.keyboard && ctx.keyboard->IsKeyPressed(VK_SPACE) &&
        ctx.dashCooldownTimer <= 0.f)
        return EPlayerStateId::Dash;

    return EPlayerStateId::Idle;
}
