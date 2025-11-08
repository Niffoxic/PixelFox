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
        //~ turret stays idle
        rigidBody->SetVelocity({ 0.f, 0.f });
    }

    if (!m_pTarget) return;

    const float distSq = DistanceFromPlayer();
    const float attack = m_nMaxShootDistance;
    const float attackSq = attack * attack;

    //~ faces towards the target
    const FVector2D aimDir = DirectionToTarget();
    if (m_pAnimStateMachine)
    {
        const CharacterState idleFace = PickIdleFromDir(aimDir);
        m_pAnimStateMachine->TransitionTo(ToString(idleFace));
    }

    if (rigidBody)
    {
        rigidBody->SetRotation(std::atan2(aimDir.y, aimDir.x));
    }

    //~ out of range
    if (distSq > attackSq) return;

    //~ in range
    if (m_pProjectile && m_nFireTimer <= 0.0f)
    {
        FVector2D spawn = rigidBody ? rigidBody->GetPosition() : FVector2D{ 0.f, 0.f };
        spawn += m_muzzleOffset;

        if (m_pProjectile->Fire(spawn, aimDir))
        {
            //~ raise attack callback with direction
            EAttackDirection dir = DirToAttackDirection(aimDir);
            if (dir == EAttackDirection::Invalid) dir = EAttackDirection::Right;
            FireAttack(dir);
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
