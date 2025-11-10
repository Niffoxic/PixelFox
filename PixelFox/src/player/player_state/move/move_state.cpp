#include "move_state.h"

#include "player/player.h"

using namespace pixel_game;

void PlayerStateMove::OnEnter(PlayerContext& ctx)
{
    const int oct = ToOctant(ctx, ResolveFacing(ctx));
    switch (oct)
    {
    case 0: ctx.anim->TransitionTo(ToString(CharacterState::WALK_RIGHT));       break;
    case 1: ctx.anim->TransitionTo(ToString(CharacterState::WALK_UP_RIGHT));    break;
    case 2: ctx.anim->TransitionTo(ToString(CharacterState::WALK_UP));          break;
    case 3: ctx.anim->TransitionTo(ToString(CharacterState::WALK_UP_LEFT));     break;
    case 4: ctx.anim->TransitionTo(ToString(CharacterState::WALK_LEFT));        break;
    case 5: ctx.anim->TransitionTo(ToString(CharacterState::WALK_DOWN_LEFT));   break;
    case 6: ctx.anim->TransitionTo(ToString(CharacterState::WALK_DOWN));        break;
    case 7: ctx.anim->TransitionTo(ToString(CharacterState::WALK_DOWN_RIGHT));  break;
    default: ctx.anim->TransitionTo(ToString(CharacterState::WALK_RIGHT));      break;
    }
}

EPlayerStateId pixel_game::PlayerStateMove::Tick(PlayerContext& ctx)
{
    if (ctx.dir.x == 0.f && ctx.dir.y == 0.f)
        return EPlayerStateId::Idle;

    if (auto* rb = ctx.self->GetPlayerBody()->GetRigidBody2D())
    {
        const FVector2D v = ctx.dir * ctx.movementSpeed * ctx.dt;
        rb->m_transform.Position += v;
    }

    ctx.lastNonZeroDir = ctx.dir;

    const int oct = ToOctant(ctx, ctx.dir);
    switch (oct)
    {
    case 0: ctx.anim->TransitionTo(ToString(CharacterState::WALK_RIGHT)); break;
    case 1: ctx.anim->TransitionTo(ToString(CharacterState::WALK_UP_RIGHT)); break;
    case 2: ctx.anim->TransitionTo(ToString(CharacterState::WALK_UP)); break;
    case 3: ctx.anim->TransitionTo(ToString(CharacterState::WALK_UP_LEFT)); break;
    case 4: ctx.anim->TransitionTo(ToString(CharacterState::WALK_LEFT)); break;
    case 5: ctx.anim->TransitionTo(ToString(CharacterState::WALK_DOWN_LEFT)); break;
    case 6: ctx.anim->TransitionTo(ToString(CharacterState::WALK_DOWN)); break;
    case 7: ctx.anim->TransitionTo(ToString(CharacterState::WALK_DOWN_RIGHT)); break;
    default: ctx.anim->TransitionTo(ToString(CharacterState::WALK_RIGHT)); break;
    }

    if (ctx.keyboard && ctx.keyboard->IsKeyPressed(VK_SPACE) &&
        ctx.dashCooldownTimer <= 0.f)
        return EPlayerStateId::Dash;

    return EPlayerStateId::Move;
}
