#include "pch.h"
#include "chase_ai.h"

using namespace pixel_game;
using namespace pixel_engine;

bool ChaseAI::Init(_In_ PEISprite* aiBody)
{
    m_pBody       = aiBody;
    m_bActive     = (m_pBody != nullptr);
    m_desiredDirection = { 0.f, 0.f };
    m_desiredSpeed = 0.f;
    return m_bActive;
}

void ChaseAI::Update(_In_ float deltaTime)
{
    if (!m_bActive || !m_pBody || deltaTime <= 0.0f) return;

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

    auto* rb = m_pBody->GetRigidBody2D();
    if (!rb) return;

    if (m_desiredSpeed > 0.0f && (m_desiredDirection.x != 0.0f || m_desiredDirection.y != 0.0f))
    {
        FVector2D pos = rb->GetPosition();
        pos.x += m_desiredDirection.x * m_desiredSpeed * deltaTime;
        pos.y += m_desiredDirection.y * m_desiredSpeed * deltaTime;

        rb->m_transform.Position = pos;
    }
}

bool ChaseAI::Release()
{
    m_pBody = nullptr;
    m_pTarget = nullptr;
    m_bActive = false;
    m_nLifeRemaining = -1.0f;
    m_desiredDirection = { 0.f, 0.f };
    m_desiredSpeed = 0.f;
    return true;
}

bool ChaseAI::Kill()
{
    m_bActive = false;
    return true;
}

bool ChaseAI::SetLifeSpan(_In_ float seconds)
{
    m_nLifeRemaining = seconds;
    return true;
}

void ChaseAI::SetTarget(_In_ PEISprite* target)
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

void ChaseAI::SetActive(_In_ bool flag)
{
    m_bActive = flag;
}

bool ChaseAI::IsActive() const
{
    return m_bActive;
}

float ChaseAI::DistanceFromPlayer() const
{
    if (!m_pBody || !m_pTarget) return FLT_MAX;

    auto* rbA = m_pBody->GetRigidBody2D();
    auto* rbB = m_pTarget->GetRigidBody2D();
    if (!rbA || !rbB) return FLT_MAX;

    const FVector2D a = rbA->GetPosition();
    const FVector2D b = rbB->GetPosition();
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

FVector2D ChaseAI::DirectionToTarget() const
{
    if (!m_pBody || !m_pTarget) return { 0.f, 0.f };

    auto* rbA = m_pBody->GetRigidBody2D();
    auto* rbB = m_pTarget->GetRigidBody2D();
    if (!rbA || !rbB) return { 0.f, 0.f };

    const FVector2D a = rbA->GetPosition();
    const FVector2D b = rbB->GetPosition();
    return NormalizeSafe({ b.x - a.x, b.y - a.y });
}

void ChaseAI::OnTargetAcquired(_In_ PEISprite* target)
{

}

void ChaseAI::OnTargetLost()
{
    m_desiredDirection = { 0.f, 0.f };
    m_desiredSpeed = 0.f;
}

void ChaseAI::UpdateAIDecision()
{
    if (!m_pBody || !m_pTarget)
    {
        m_desiredDirection = { 0.f, 0.f };
        m_desiredSpeed = 0.f;
        return;
    }

    const FVector2D dir = DirectionToTarget();

    if (m_stopDistance > 0.0f)
    {
        const float dist = DistanceFromPlayer();
        if (dist <= m_stopDistance)
        {
            m_desiredDirection = { 0.f, 0.f };
            m_desiredSpeed = 0.f;
            return;
        }
    }

    m_desiredDirection = dir;
    m_desiredSpeed = m_moveSpeed;
}

FVector2D pixel_game::ChaseAI::NormalizeSafe(const FVector2D& v) const
{
    const float dotSq = v.x * v.x + v.y * v.y;
    if (dotSq <= 1e-12f) return { 0.f, 0.f };

    const float inv = 1.0f / std::sqrt(dotSq);

    return { v.x * inv, v.y * inv };
}
