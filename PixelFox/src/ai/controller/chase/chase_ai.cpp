#include "pch.h"
#include "chase_ai.h"

#include "fox_math/math.h"
#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_game;
using namespace pixel_engine;


_Use_decl_annotations_
bool ChaseAI::Init(const PE_AI_CONTROLLER_DESC& desc)
{
    m_pBody = desc.pAiBody;
    m_pAnimStateMachine = desc.pAnimStateMachine;
    m_bActive = (m_pBody != nullptr);
    m_desiredDirection = { 0.f, 0.f };
    m_nDesiredSpeed = 0.f;
    m_bInAttackRange = false;
    m_bAttacking = false;
    m_eCurrentAnim = CharacterState::IDLE_LEFT;

    SetOnAttack(desc.fnOnAttackCallback);
    SetOnCantAttack(desc.fnOnCantAttack);

    if (m_pAnimStateMachine)
    {
        m_pAnimStateMachine->TransitionTo(ToString(m_eCurrentAnim));
    }

    return m_bActive;
}

_Use_decl_annotations_
void ChaseAI::Update(float deltaTime)
{
    if (!m_bActive || !m_pBody || deltaTime <= 0.0f) return;

    if (m_nLifeRemaining >= 0.0f)
    {
        m_nLifeRemaining -= deltaTime;
        if (m_nLifeRemaining <= 0.0f) { m_bActive = false; return; }
    }

    UpdateAIDecision();

    const bool hasTarget = (m_pTarget != nullptr);
    if (hasTarget)
    {
        const float dist = DistanceFromPlayer();
        if (dist <= m_nAttackDistance)
        {
            if (!m_bInAttackRange)
            {
                m_bInAttackRange = true;
                const FVector2D dir = DirectionToTarget();
                TransitionToIfChanged(PickAttackFromDir(dir));
                m_bAttacking = true;
                const EAttackDirection d = DirToAttackDirection(dir);
                FireAttack(d == EAttackDirection::Invalid ? EAttackDirection::Right : d);
            }
            else
            {
                const FVector2D dir = DirectionToTarget();
                TransitionToIfChanged(PickAttackFromDir(dir));
            }

            m_nDesiredSpeed = 0.f;
            if (auto* rb = m_pBody->GetRigidBody2D()) rb->SetVelocity({ 0.f, 0.f });
        }
        else
        {
            if (m_bInAttackRange || m_bAttacking)
            {
                FireStopAttack();
            }
            m_bInAttackRange = false;
            m_bAttacking = false;
        }
    }

    if (m_bAttacking)
    {
        m_nDesiredSpeed = 0.f;
        if (auto* rb = m_pBody->GetRigidBody2D())
            rb->SetVelocity({ 0.f, 0.f });
    }
    else
    {
        auto* rigidBody = m_pBody->GetRigidBody2D();
        if (!rigidBody) return;

        if (m_nDesiredSpeed > 0.0f && (m_desiredDirection.x != 0.0f || m_desiredDirection.y != 0.0f))
        {
            const FVector2D dir = m_desiredDirection.SafeNormalized();
            FVector2D pos = rigidBody->GetPosition();
            pos.x += dir.x * (m_nDesiredSpeed * deltaTime);
            pos.y += dir.y * (m_nDesiredSpeed * deltaTime);
            rigidBody->m_transform.Position = pos;
        }
    }

    const bool moving = (m_nDesiredSpeed > 0.0f) && (m_desiredDirection.x != 0.f || m_desiredDirection.y != 0.f);
    UpdateAnimationState(moving);
}

_Use_decl_annotations_
bool ChaseAI::Release()
{
    m_pBody = nullptr;
    m_pTarget = nullptr;
    m_bActive = false;
    m_nLifeRemaining = -1.0f;
    m_desiredDirection = { 0.f, 0.f };
    m_nDesiredSpeed = 0.f;
    m_bInAttackRange = false;
    m_bAttacking = false;
    m_eCurrentAnim = CharacterState::IDLE_LEFT;
    return true;
}

_Use_decl_annotations_
bool ChaseAI::Kill()
{
    m_bActive = false;
    m_nLifeRemaining = 0.f;
    return true;
}

_Use_decl_annotations_
void ChaseAI::SetLifeSpan(float seconds)
{
    m_nLifeRemaining = seconds;
}

_Use_decl_annotations_
void ChaseAI::SetTarget(PEISprite* target)
{
    if (target == m_pTarget) return;

    if (target)
    {
        m_pTarget = target;
        OnTargetAcquired(target);
    }
    else
    {
        m_pTarget = nullptr;
        OnTargetLost();
    }
}

_Use_decl_annotations_
void ChaseAI::SetActive(bool flag)
{
    m_bActive = flag;
}

_Use_decl_annotations_
bool ChaseAI::IsActive() const
{
    return m_bActive;
}

_Use_decl_annotations_
void ChaseAI::OnTargetAcquired(PEISprite* target)
{
}

void ChaseAI::OnTargetLost()
{
    m_desiredDirection = { 0.f, 0.f };
    m_nDesiredSpeed = 0.f;
    m_bInAttackRange = false;
    m_bAttacking = false;
}

void ChaseAI::UpdateAIDecision()
{
    if (!m_pBody || !m_pTarget)
    {
        m_desiredDirection = { 0.f, 0.f };
        m_nDesiredSpeed = 0.f;
        return;
    }

    const FVector2D dir = DirectionToTarget();

    if (m_nStopDistance > 0.0f)
    {
        const float dist = DistanceFromPlayer();
        if (dist <= m_nStopDistance)
        {
            m_desiredDirection = { 0.f, 0.f };
            m_nDesiredSpeed = 0.f;
            return;
        }
    }

    m_desiredDirection = dir.SafeNormalized();
    m_nDesiredSpeed = m_nMoveSpeed;
}

void ChaseAI::UpdateAnimationState(bool moving)
{
    if (!m_pAnimStateMachine) return;

    if (m_pTarget && m_nAttackDistance > 0.0f)
    {
        const float dist = DistanceFromPlayer();
        if (dist <= m_nAttackDistance)
        {
            const FVector2D dir = DirectionToTarget();
            const CharacterState next = PickAttackFromDir(dir);
            TransitionToIfChanged(next);
            return;
        }
    }

    const CharacterState next = PickWalkOrIdleFromDir(m_desiredDirection, moving);
    TransitionToIfChanged(next);
}

void pixel_game::ChaseAI::FireAttack(_In_ EAttackDirection direction)
{
    if (m_fnOnAttackCallback) m_fnOnAttackCallback(*this, direction);
}

void pixel_game::ChaseAI::FireStopAttack()
{
    if (m_fnOnStopCallback) m_fnOnStopCallback(*this);
}

_Use_decl_annotations_
void pixel_game::ChaseAI::TransitionToIfChanged(CharacterState next)
{
    if (m_pAnimStateMachine)
    {
        if (next == m_eCurrentAnim)
        {
            if (auto* anim = m_pAnimStateMachine->GetCurrentAnim())
            {
                if (anim->IsCycleComplete()) m_pAnimStateMachine->TransitionTo(ToString(next));
            }
            return;
        }
        m_eCurrentAnim = next;
        m_pAnimStateMachine->TransitionTo(ToString(next));
        return;
    }

    if (next == m_eCurrentAnim) return;
    m_eCurrentAnim = next;
}
