#include "pch.h"
#include "chase_ai.h"

using namespace pixel_game;
using namespace pixel_engine;

_Use_decl_annotations_
bool ChaseAI::Init(const PE_AI_CONTROLLER_DESC& desc)
{
    m_pBody              = desc.pAiBody;
    m_pAnimStateMachine  = desc.pAnimStateMachine;
    m_bActive            = (m_pBody != nullptr);
    m_desiredDirection   = { 0.f, 0.f };
    m_nDesiredSpeed      = 0.f;
    return m_bActive;
}

_Use_decl_annotations_
void ChaseAI::Update(float deltaTime)
{
    if (!m_bActive || !m_pBody || deltaTime <= 0.0f) return;

    //~ if ai on timer
    if (m_nLifeRemaining >= 0.0f)
    {
        m_nLifeRemaining -= deltaTime;

        if (m_nLifeRemaining <= 0.0f)
        {
            m_bActive = false;
            return;
        }
    }

    UpdateAIDecision();

    //~ updates ai position
    auto* rigidBody = m_pBody->GetRigidBody2D();
    if (!rigidBody) return;

    if (m_nDesiredSpeed > 0.0f &&
        (m_desiredDirection.x != 0.0f || m_desiredDirection.y != 0.0f))
    {
        FVector2D pos = rigidBody->GetPosition();
        pos.x += m_desiredDirection.x * m_nDesiredSpeed * deltaTime;
        pos.y += m_desiredDirection.y * m_nDesiredSpeed * deltaTime;

        rigidBody->m_transform.Position = pos;
    }
}

_Use_decl_annotations_
bool ChaseAI::Release()
{
    m_pBody             = nullptr;
    m_pTarget           = nullptr;
    m_bActive           = false;
    m_nLifeRemaining    = -1.0f;
    m_desiredDirection  = { 0.f, 0.f };
    m_nDesiredSpeed     = 0.f;
    return true;
}

_Use_decl_annotations_
bool ChaseAI::Kill()
{
    m_bActive        = false;
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
    // TODO: Callback to the owner for animation of other effect
}

void ChaseAI::OnTargetLost()
{
    m_desiredDirection = { 0.f, 0.f };
    m_nDesiredSpeed    = 0.f;
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

    m_desiredDirection = dir;
    m_nDesiredSpeed = m_nMoveSpeed;
}
