#pragma once

#include "interface_controller.h"
#include <sal.h>
#include <cmath>
#include <cfloat>

namespace pixel_game
{
    class ChaseAI final : public IAIController
    {
    public:
        ChaseAI() = default;
        ~ChaseAI() override = default;

        // IAIController
        bool Init   (_In_ pixel_engine::PEISprite* aiBody) override;
        void Update (_In_ float deltaTime)                 override;
        bool Release()                                     override;
        bool Kill   ()                                     override;
        bool SetLifeSpan(_In_ float seconds)               override;

        void SetTarget(_In_ pixel_engine::PEISprite* target) override;
        void SetActive(_In_ bool flag)                       override;
        bool IsActive() const                                override;

        _NODISCARD pixel_engine::PEISprite* GetBody  () const override { return m_pBody; }
        _NODISCARD pixel_engine::PEISprite* GetTarget() const override { return m_pTarget; }

        float     DistanceFromPlayer() const override;
        FVector2D DirectionToTarget () const override;

        bool      HasTarget() const override { return m_pTarget != nullptr; }

        void OnTargetAcquired(_In_ pixel_engine::PEISprite* target) override;
        void OnTargetLost()                                         override;

    protected:
        void UpdateAIDecision() override;

    private:
        FVector2D NormalizeSafe(const FVector2D& v) const;

    private:
        pixel_engine::PEISprite* m_pBody  { nullptr };
        pixel_engine::PEISprite* m_pTarget{ nullptr };

        bool  m_bActive       { true };
        float m_nLifeRemaining{ -1.0f };

        FVector2D m_desiredDirection{ 0.f, 0.f };
        float     m_desiredSpeed    { 0.f };

        float m_moveSpeed   { 4.0f };
        float m_stopDistance{ 0.0f };
    };
}
