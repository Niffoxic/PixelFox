#pragma once

#include "ai/controller/interface_controller.h"
#include "ai/projectile/interface_projectile.h"
#include <sal.h>
#include <cfloat>
#include <cmath>

namespace pixel_game
{
    class TurretAI final : public IAIController
    {
    public:
        TurretAI() = default;
        ~TurretAI() override = default;

        _NODISCARD _Check_return_
        bool Init(_In_ const PE_AI_CONTROLLER_DESC& desc) override;

        void Update(_In_ float deltaTime) override;

        _NODISCARD _Check_return_
        bool Release() override;

        _NODISCARD _Check_return_
        bool Kill() override;

        void SetLifeSpan(_In_ float seconds)                       override;
        void SetTarget  (_In_opt_ pixel_engine::PEISprite* target) override;
        void SetActive  (_In_ bool flag)                           override;

        _NODISCARD _Check_return_
        bool IsActive() const override;

        _NODISCARD _Check_return_ _Ret_maybenull_
        pixel_engine::PEISprite* GetBody  () const override { return m_pBody;   }

        _NODISCARD _Check_return_ _Ret_maybenull_
        pixel_engine::PEISprite* GetTarget() const override { return m_pTarget; }

        _NODISCARD _Check_return_
        bool HasTarget() const override { return m_pTarget != nullptr; }

        void OnTargetAcquired(_In_ pixel_engine::PEISprite* target) override;
        void OnTargetLost() override;

        //~ Turret configuration
        void SetProjectile      (_In_opt_ IProjectile* projectile) noexcept;
        void SetFireCooldown    (_In_ float seconds)               noexcept;
        void SetMuzzleOffset    (_In_ const FVector2D& off)        noexcept;
        void SetMaxShootDistance(_In_ float distance)              noexcept;

    protected:
        void UpdateAIDecision() override;

    private:
        pixel_engine::AnimSateMachine* m_pAnimStateMachine  { nullptr };
        pixel_engine::PEISprite*       m_pBody              { nullptr };
        pixel_engine::PEISprite*       m_pTarget            { nullptr };
        IProjectile*                   m_pProjectile        { nullptr };

        bool        m_bActive          { true     };
        float       m_nLifeRemaining   { -1.0f    };
        float       m_nFireCooldown    { 0.6f     };
        float       m_nFireTimer       { 0.0f     };
        float       m_nMaxShootDistance{ 40.f     };
        FVector2D   m_muzzleOffset     { 0.f, 0.f };
    };
}
