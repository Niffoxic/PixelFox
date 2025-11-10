#pragma once

#include "fox_math/vector.h"

#include <cstdint>
#include <string_view>
#include "pixel_engine/render_manager/components/animator/anim_state.h"
#include "pixel_engine/window_manager/inputs/keyboard_inputs.h"

namespace pixel_engine { class AnimSateMachine; class PEKeyboardInputs; }
namespace pixel_game   { class PlayerCharacter; }

namespace pixel_game
{
    enum class EPlayerStateId : uint8_t
    {
        Idle = 0,
        Move,
        Dash,
        Attack1,
        Attack2,
        Hurt,
        Death
    };

    enum class EInterruptPolicy : uint8_t
    {
        Interruptible,
        OnlyByHurtOrDeath,      // for  hurt or death
        Uninterruptible         // special attacks
    };

    struct PlayerContext
    {
        PlayerCharacter*               self{ nullptr };
        pixel_engine::AnimSateMachine* anim{ nullptr };

        //~ Input
        const pixel_engine::PEKeyboardInputs* keyboard{ nullptr };
        float                                 dt{ 0.0f };

        //~ Kinetics
        float& movementSpeed;
        float& dashForce;

        //~ Dash timing
        float& dashCooldown;
        float& dashCooldownTimer;
        float& dashDuration;
        float& dashTimer;

        //~ Special Attack 
        float& specialCooldown;
        float& specialCooldownTimer;

        // Directions
        FVector2D& dir;             //~ current input direction
        FVector2D& lastNonZeroDir;  //~ last facing
    };

    /// <summary>
    /// Manages Player State along with anim state
    /// </summary>
    class IPlayerState
    {
    public:
        virtual ~IPlayerState() = default;

        virtual EPlayerStateId Id() const noexcept = 0;
        virtual std::string_view Name() const noexcept = 0;
        
        virtual void OnEnter(PlayerContext& ctx) {}
        virtual void OnExit(PlayerContext& ctx) {}

        _NODISCARD
        virtual EPlayerStateId Tick(PlayerContext& ctx) = 0;

        _NODISCARD
        virtual bool IsOneShot() const noexcept { return false; }

        _NODISCARD
        virtual EInterruptPolicy InterruptPolicy() const noexcept
        {
            return EInterruptPolicy::Interruptible;
        }

        _NODISCARD
        virtual bool ControlsMovementDirectly() const noexcept
        {
            return false;
        }
    };

    class PlayerStateBase : public IPlayerState
    {
    public:
        static FVector2D ResolveFacing(const PlayerContext& ctx) noexcept;
        static int ToOctant(const PlayerContext& ctx, const FVector2D& dir) noexcept;
    };

} // namespace pixel_game
