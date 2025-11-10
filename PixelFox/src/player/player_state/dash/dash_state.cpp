#include "dash_state.h"

#include "player/player.h"

using namespace pixel_game;

void PlayerStateDash::OnEnter(PlayerContext& ctx)
{
    FVector2D dashDir = ResolveFacing(ctx);
    const int oct = ToOctant(ctx, dashDir);

    if (auto* rb = ctx.self->GetPlayerBody()->GetRigidBody2D())
        rb->AddVelocity(dashDir * ctx.dashForce);

    std::string cacheCS;
    switch (oct)
    {
    case 0: ctx.anim->TransitionTo(ToString(CharacterState::DASH_RIGHT)); break;
    case 1: ctx.anim->TransitionTo(ToString(CharacterState::DASH_UP_RIGHT)); break;
    case 2: ctx.anim->TransitionTo(ToString(CharacterState::DASH_UP)); break;
    case 3: ctx.anim->TransitionTo(ToString(CharacterState::DASH_UP_LEFT)); break;
    case 4: ctx.anim->TransitionTo(ToString(CharacterState::DASH_LEFT)); break;
    case 5: ctx.anim->TransitionTo(ToString(CharacterState::DASH_DOWN_LEFT)); break;
    case 6: ctx.anim->TransitionTo(ToString(CharacterState::DASH_DOWN)); break;
    case 7: ctx.anim->TransitionTo(ToString(CharacterState::DASH_DOWN_RIGHT)); break;
    default: ctx.anim->TransitionTo(ToString(CharacterState::DASH_RIGHT)); break;
    }

    ctx.lastNonZeroDir = dashDir;
    ctx.dashCooldownTimer = ctx.dashCooldown;
    ctx.dashTimer = ctx.dashDuration;

    //~ dash related state
    const uint64_t mySerial = ctx.self->BumpDashSerial();
    ctx.self->SetDashAnimDone(false);

    auto state = ctx.anim->GetCurrentState();
    ctx.anim->SetOnExitCallback(state, [self = ctx.self, mySerial]()
        {
            if (self && self->CurrentDashSerial() == mySerial)
            {
                self->SetDashAnimDone(true);
            }
        });

}

EPlayerStateId PlayerStateDash::Tick(PlayerContext& ctx)
{
    if (ctx.dashTimer > 0.f)
        ctx.dashTimer -= ctx.dt;

    const bool finished = ctx.self->IsDashAnimDone() || (ctx.dashTimer <= 0.f);

    if (finished)
    {
        if (ctx.dir.x != 0.f || ctx.dir.y != 0.f)
            return EPlayerStateId::Move;
        return EPlayerStateId::Idle;
    }

    return EPlayerStateId::Dash;
}

void PlayerStateDash::OnExit(PlayerContext& ctx)
{
    ctx.self->SetDashAnimDone(false);
}