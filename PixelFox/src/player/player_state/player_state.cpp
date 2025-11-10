#include "player_state.h"
#include "fox_math/math.h"

FVector2D pixel_game::PlayerStateBase::ResolveFacing(const PlayerContext& ctx) noexcept
{
    FVector2D d = ((ctx.dir.x != 0.f) ||
                    (ctx.dir.y != 0.f)) ? 
        ctx.dir : ctx.lastNonZeroDir;
    return d;    
}

int pixel_game::PlayerStateBase::ToOctant(const PlayerContext& ctx, const FVector2D& dir) noexcept
{
    float eps = 1e-6f; //~ fixes wierd bug where even tho it should be zero!
    if (std::fabs(dir.x) < eps && std::fabs(dir.y) < eps)
    {
        return -1;
    }

    float ang = std::atan2(-dir.y, dir.x);
    float pi = 3.14159265358979323846;
    float oct = pi / 4.0f;

    if (ang < 0.0f) ang += 2.0f * pi;

    return static_cast<int>(std::floor((ang + oct * 0.5f) / oct)) & 7;
}
