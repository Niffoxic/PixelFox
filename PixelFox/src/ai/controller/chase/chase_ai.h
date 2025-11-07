#pragma once

#include "ai/controller/interface_controller.h"
#include <sal.h>
#include <cmath>
#include <cfloat>

#include "world/state/character_state.h"

namespace pixel_game
{
    class ChaseAI final : public IAIController
    {
    public:
         ChaseAI() = default;
        ~ChaseAI() override = default;

        _NODISCARD _Check_return_
        bool Init(_In_ const PE_AI_CONTROLLER_DESC& desc) override;

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

        //~ attak logic
        void SetOnAttack(_In_opt_ AttackCallbackType cb) override { m_fnOnAttackCallback = std::move(cb); }
        _NODISCARD bool HasOnAttack() const override { return static_cast<bool>(m_fnOnAttackCallback); }
        void FireAttack(_In_ EAttackDirection direction) override;

        void SetOnCantAttack(_In_opt_ CantAttackCallbackType cb) override { m_fnOnStopCallback = std::move(cb); }
        void FireStopAttack() override;

        void  SetAttackDistance(_In_ float d) { m_nAttackDistance = (d < 0.f ? 0.f : d); }
        _NODISCARD _Check_return_ float GetAttackDistance() const { return m_nAttackDistance; }

    protected:
        void UpdateAIDecision() override;
        void UpdateAnimationState(bool moving);
        
        //~ helpers
        void TransitionToIfChanged(_In_ CharacterState next);

    private:
        pixel_engine::AnimSateMachine* m_pAnimStateMachine{ nullptr };
        pixel_engine::PEISprite*       m_pBody            { nullptr };
        pixel_engine::PEISprite*       m_pTarget          { nullptr };

        bool      m_bActive         { true };
        float     m_nLifeRemaining  { -1.0f };
        FVector2D m_desiredDirection{ 0.f, 0.f };
        float     m_nDesiredSpeed   { 0.f };
        float     m_nMoveSpeed      { 4.0f };
        float     m_nStopDistance   { 0.0f };

        bool            m_bAttackSpent{ false };
        float           m_nAttackSpeed   { 1.f };
        float           m_nAttackDistance{ 3.0f };                 
        bool            m_bInAttackRange { false };
        bool            m_bAttacking     { false };
        CharacterState  m_eCurrentAnim   { CharacterState::IDLE_LEFT };
        
        //~ callbacks
        AttackCallbackType      m_fnOnAttackCallback{ nullptr };
        CantAttackCallbackType  m_fnOnStopCallback  { nullptr };
    };
}
