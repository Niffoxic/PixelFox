#pragma once

#include "ai/controller/interface_controller.h"
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

        _NODISCARD _Check_return_
        bool Init(_In_ pixel_engine::PEISprite* aiBody) override;

        void Update(_In_ float deltaTime) override;

        _NODISCARD _Check_return_
        bool Release() override;

        _NODISCARD _Check_return_
        bool Kill() override;

        void SetLifeSpan(_In_ float seconds) override;

        void SetTarget(_In_opt_ pixel_engine::PEISprite* target) override;

        void SetActive(_In_ bool flag) override;

        _NODISCARD _Check_return_
        bool IsActive() const override;

        _NODISCARD _Check_return_ _Ret_maybenull_
        pixel_engine::PEISprite* GetBody  () const override { return m_pBody; }

        _NODISCARD _Check_return_ _Ret_maybenull_
        pixel_engine::PEISprite* GetTarget() const override { return m_pTarget; }

        _NODISCARD _Check_return_
        bool HasTarget() const override { return m_pTarget != nullptr; }

        void OnTargetAcquired(_In_ pixel_engine::PEISprite* target) override;
        void OnTargetLost() override;

    protected:
        void UpdateAIDecision() override;

    private:
        pixel_engine::PEISprite* m_pBody  { nullptr };
        pixel_engine::PEISprite* m_pTarget{ nullptr };

        bool      m_bActive         { true };
        float     m_nLifeRemaining  { -1.0f };
        FVector2D m_desiredDirection{ 0.f, 0.f };
        float     m_nDesiredSpeed   { 0.f };
        float     m_nMoveSpeed      { 4.0f };
        float     m_nStopDistance   { 0.0f };
    };
}
