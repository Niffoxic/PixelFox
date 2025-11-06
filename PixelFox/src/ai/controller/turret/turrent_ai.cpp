#include "pch.h"
#include "turret_ai.h"

using namespace pixel_game;
using namespace pixel_engine;

_Use_decl_annotations_
bool TurretAI::Init(PEISprite* aiBody)
{
    m_pBody      = aiBody;
    m_bActive    = (m_pBody != nullptr);
    m_nFireTimer = 0.0f;

    if (m_pBody)
    {
        if (auto* rigidBody = m_pBody->GetRigidBody2D())
        {
            rigidBody->SetVelocity    ({ 0.f, 0.f });
            rigidBody->SetAcceleration({ 0.f, 0.f });
        }
    }
    return m_bActive;
}

_Use_decl_annotations_
void TurretAI::Update(float deltaTime)
{
    if (!m_bActive || !m_pBody || deltaTime <= 0.0f) return;

    //~ if turret on a timer
    if (m_nLifeRemaining >= 0.0f)
    {
        m_nLifeRemaining -= deltaTime;
        if (m_nLifeRemaining <= 0.0f) { m_bActive = false; return; }
    }

    //~ fire cooldown
    if (m_nFireTimer > 0.0f) m_nFireTimer -= deltaTime;

    UpdateAIDecision();

    if (m_pProjectile && m_pProjectile->IsActive())
    {
        m_pProjectile->Update(deltaTime);
    }
}

_Use_decl_annotations_
bool TurretAI::Release()
{
    m_pBody          = nullptr;
    m_pTarget        = nullptr;
    m_pProjectile    = nullptr;
    m_bActive        = false;
    m_nLifeRemaining = -1.0f;
    m_nFireTimer     = 0.0f;
    return true;
}

_Use_decl_annotations_
bool TurretAI::Kill()
{
    m_bActive = false;
    // TODO: add callback to the owner
    return true;
}

_Use_decl_annotations_
void TurretAI::SetLifeSpan(float seconds)
{
    m_nLifeRemaining = seconds;
}

_Use_decl_annotations_
void TurretAI::SetTarget(PEISprite* target)
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
void TurretAI::SetActive(bool flag)
{
    m_bActive = flag;

    if (!m_bActive && m_pBody)
    {
        if (auto* rigidBody = m_pBody->GetRigidBody2D())
        {
            rigidBody->SetVelocity({ 0.f, 0.f });
        }
    }
}

_Use_decl_annotations_
bool TurretAI::IsActive() const
{
    return m_bActive;
}

_Use_decl_annotations_
void TurretAI::OnTargetAcquired(PEISprite* target)
{
    //~ TODO: Add Callback to the owner
}

void TurretAI::OnTargetLost() 
{
    //~ TODO: Add Callback to the owner
}

void TurretAI::UpdateAIDecision()
{
    if (!m_pBody) return;

    auto* rigidBody = m_pBody->GetRigidBody2D();

    if (rigidBody) 
    {
        rigidBody->SetVelocity({ 0.f, 0.f });
    } 

    if (!m_pTarget) return;

    const float distance = DistanceFromPlayer();

    if (distance > m_nMaxShootDistance)
    {
        return; 
    }

    const FVector2D dir = DirectionToTarget();
    
    if (!(rigidBody && (dir.x != 0.f || dir.y != 0.f))) return;

    rigidBody->SetRotation(std::atan2(dir.y, dir.x));

    if (m_pProjectile && m_nFireTimer <= 0.0f)
    {
        FVector2D spawn = rigidBody->GetPosition();
        spawn += m_muzzleOffset;

        if (m_pProjectile->Fire(spawn, dir))
        {
            m_nFireTimer = m_nFireCooldown;
        }
    }
}

_Use_decl_annotations_
void pixel_game::TurretAI::SetProjectile(IProjectile* projectile) noexcept
{
    m_pProjectile = projectile;
}

_Use_decl_annotations_
void pixel_game::TurretAI::SetFireCooldown(float seconds) noexcept
{
    m_nFireCooldown = (seconds >= 0.f ? seconds : 0.f);
}

_Use_decl_annotations_
void pixel_game::TurretAI::SetMuzzleOffset(const FVector2D& off) noexcept
{
    m_muzzleOffset = off;
}

_Use_decl_annotations_
void pixel_game::TurretAI::SetMaxShootDistance(float distance) noexcept
{
    m_nMaxShootDistance = (distance >= 0.f ? distance : 40.f);
}
