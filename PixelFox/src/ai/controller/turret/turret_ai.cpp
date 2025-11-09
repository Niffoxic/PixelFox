#include "pch.h"
#include "turret_ai.h"

using namespace pixel_game;
using namespace pixel_engine;

_Use_decl_annotations_
bool TurretAI::Init(const PE_AI_CONTROLLER_DESC& desc)
{
    m_pAnimStateMachine = desc.pAnimStateMachine;
    m_pBody             = desc.pAiBody;
    m_bActive           = (m_pBody != nullptr);
    m_nFireTimer        = 0.0f;

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
        if (m_nLifeRemaining <= 0.0f) 
        {
            m_bActive = false; 
            return; 
        }
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

    const float distSq = DistanceFromPlayer();
    const float attackSq = m_nMaxShootDistance * m_nMaxShootDistance;

    const FVector2D aimDir = DirectionToTarget();

    if (m_pAnimStateMachine)
    {
        const CharacterState idleFace = PickIdleFromDir(aimDir);
        m_pAnimStateMachine->TransitionTo(ToString(idleFace));
    }

    float turretAngle = std::atan2(aimDir.y, aimDir.x);
    if (rigidBody)
    {
        rigidBody->SetRotation(turretAngle);
    }

    if (distSq > attackSq) return;

    if (m_pProjectile && m_nFireTimer <= 0.0f)
    {
        FVector2D dir = aimDir;
        const float len2 = dir.x * dir.x + dir.y * dir.y;
        if (len2 > 1e-12f)
        {
            const float invLen = 1.0f / std::sqrt(len2);
            dir.x *= invLen; dir.y *= invLen;
        }
        else
        {
            dir = { std::cos(turretAngle), std::sin(turretAngle) };
        }

        const FVector2D basePos = rigidBody ? rigidBody->GetPosition() : FVector2D{ 0.f, 0.f };
        FireProjectileTowardsTarget(basePos, turretAngle, dir);
    }
}

_Use_decl_annotations_
void pixel_game::TurretAI::FireProjectileTowardsTarget(
    const FVector2D& basePos
    ,float angleRadians,
    const FVector2D& aimDirNorm)
{
    if (!m_pProjectile) return;

    // Rotate muzzle offset by current turret angle
    const float c = std::cos(angleRadians), s = std::sin(angleRadians);
    const FVector2D muzzleWorld{
        m_muzzleOffset.x * c - m_muzzleOffset.y * s,
        m_muzzleOffset.x * s + m_muzzleOffset.y * c
    };

    FVector2D spawn = basePos + muzzleWorld;

    if (m_pProjectile->Fire(spawn, aimDirNorm))
    {
        EAttackDirection dir = DirToAttackDirection(aimDirNorm);
        if (dir == EAttackDirection::Invalid) dir = EAttackDirection::Right;
        FireAttack(dir);
        m_nFireTimer = m_nFireCooldown;
    }
    else
    {
        FireStopAttack();
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
void pixel_game::TurretAI::SetOnAttack(AttackCallbackType cb)
{
    if (cb)
    {
        m_fnOnAttackCallback = std::move(cb);
    }
}

_Use_decl_annotations_
void pixel_game::TurretAI::SetAttackDistance(float distance)
{
    m_nMaxShootDistance = distance;
}

_Use_decl_annotations_
void pixel_game::TurretAI::FireAttack(EAttackDirection direction)
{
    if (m_fnOnAttackCallback) m_fnOnAttackCallback(*this, direction);
}

_Use_decl_annotations_
void pixel_game::TurretAI::SetOnCantAttack(CantAttackCallbackType cb)
{
    if (cb) m_fnOnStopCallback = std::move(cb);
}

void pixel_game::TurretAI::FireStopAttack()
{
    if (m_fnOnStopCallback) m_fnOnStopCallback(*this);
}

_Use_decl_annotations_
bool pixel_game::TurretAI::HasOnAttack() const
{
    return static_cast<bool>(m_fnOnAttackCallback);
}

_Use_decl_annotations_
float pixel_game::TurretAI::GetAttackDistance() const
{
    return m_nMaxShootDistance;
}
