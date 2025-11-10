#include "enemy_fire_dog.h"

#include "pixel_engine/utilities/logger/logger.h"
#include <filesystem>

#include "world/events/projectile_events.h"
#include "world/events/player_events.h"

#include "pixel_engine/exceptions/base_exception.h"

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::Initialize(const PG_ENEMY_INIT_DESC& desc)
{
    SetTarget(desc.pTarget);

    if (!InitEnemyBody(desc)) return false;
    if (!InitEnemyAnimStateMachine()) return false;
    if (!InitEnemyAI(desc)) return false;

    SubscribeEvents();
    InitCollisionCallback();
    AddColliderTags();

    return true;
}

_Use_decl_annotations_
void pixel_game::EnemyFireDog::Update(float deltaTime)
{
    UpdateAnimState(deltaTime);
    UpdateAIController(deltaTime);
}

void pixel_game::EnemyFireDog::Release()
{
    m_bActive = false;
    m_pTarget = nullptr;

    m_pBody.reset();
    m_pAnimState.reset();
    m_pAIController.reset();
}

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::IsActive() const
{
    return m_bActive;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::EnemyFireDog::GetBody() const
{
    return m_pBody.get();
}

_Use_decl_annotations_
pixel_engine::AnimSateMachine* pixel_game::EnemyFireDog::GetAnimState() const
{
    return m_pAnimState.get();
}

_Use_decl_annotations_
pixel_engine::BoxCollider* pixel_game::EnemyFireDog::GetCollider() const
{
    if (m_pBody && m_pBody->GetCollider()) return m_pBody->GetCollider();
    return nullptr;
}

_Use_decl_annotations_
pixel_game::IAIController* pixel_game::EnemyFireDog::GetController() const
{
    return m_pAIController.get();
}

_Use_decl_annotations_
void pixel_game::EnemyFireDog::SetTarget(pixel_engine::PEISprite* target)
{
    m_pTarget = target;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::EnemyFireDog::GetTarget() const
{
    return m_pTarget;
}

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::HasTarget() const
{
    return m_pTarget != nullptr;
}

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::IsDead() const
{
    return m_nHealth <= 0.f;
}

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::InitEnemyBody(const PG_ENEMY_INIT_DESC& desc)
{
    m_pBody = std::make_unique<pixel_engine::QuadObject>();
    m_pBody->SetScale(desc.Scale);
    m_pBody->SetPosition(desc.SpawnPoint);
    m_pBody->SetLayer(pixel_engine::ELayer::Npc_AI);
    m_pBody->SetVisible(true);
    m_pBody->GetCollider()->SetColliderType(pixel_engine::ColliderType::Dynamic);

    std::string idle = m_szBaseFile + "Idle/left/00.png";
    m_pBody->SetTexture(idle);
    return m_pBody->Initialize();
}

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::InitEnemyAnimStateMachine()
{
    m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pBody.get());

    //~ Attack
    ValidatePathExists(m_szBaseFile + "Attack/left/");
    ValidatePathExists(m_szBaseFile + "Attack/right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_LEFT),
        m_szBaseFile + "Attack/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_RIGHT),
        m_szBaseFile + "Attack/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP), 
        m_szBaseFile + "Attack/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN),
        m_szBaseFile + "Attack/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP_LEFT), 
        m_szBaseFile + "Attack/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP_RIGHT), 
        m_szBaseFile + "Attack/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN_LEFT),
        m_szBaseFile + "Attack/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN_RIGHT),
        m_szBaseFile + "Attack/right/");

    //~ Death
    ValidatePathExists(m_szBaseFile + "Death/left/");
    ValidatePathExists(m_szBaseFile + "Death/right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_LEFT),
        m_szBaseFile + "Death/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_RIGHT),
        m_szBaseFile + "Death/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP),
        m_szBaseFile + "Death/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN),
        m_szBaseFile + "Death/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP_LEFT),
        m_szBaseFile + "Death/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP_RIGHT),
        m_szBaseFile + "Death/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN_LEFT),
        m_szBaseFile + "Death/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN_RIGHT),
        m_szBaseFile + "Death/right/");

    //~ Idle
    ValidatePathExists(m_szBaseFile + "Idle/left/");
    ValidatePathExists(m_szBaseFile + "Idle/right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_LEFT),
        m_szBaseFile + "Idle/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_RIGHT), 
        m_szBaseFile + "Idle/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP),
        m_szBaseFile + "Idle/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN),
        m_szBaseFile + "Idle/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP_LEFT),
        m_szBaseFile + "Idle/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP_RIGHT),
        m_szBaseFile + "Idle/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN_LEFT),
        m_szBaseFile + "Idle/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN_RIGHT),
        m_szBaseFile + "Idle/right/");

    //~ Hurt
    ValidatePathExists(m_szBaseFile + "Hurt/left/");
    ValidatePathExists(m_szBaseFile + "Hurt/right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_LEFT),
        m_szBaseFile + "Hurt/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_RIGHT),
        m_szBaseFile + "Hurt/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP), 
        m_szBaseFile + "Hurt/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN), 
        m_szBaseFile + "Hurt/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP_LEFT),
        m_szBaseFile + "Hurt/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP_RIGHT),
        m_szBaseFile + "Hurt/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN_LEFT),
        m_szBaseFile + "Hurt/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN_RIGHT),
        m_szBaseFile + "Hurt/right/");

    //~ Walk
    ValidatePathExists(m_szBaseFile + "Walk/left/");
    ValidatePathExists(m_szBaseFile + "Walk/right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_LEFT),
        m_szBaseFile + "Walk/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_RIGHT),
        m_szBaseFile + "Walk/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP),
        m_szBaseFile + "Walk/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN), 
        m_szBaseFile + "Walk/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP_LEFT),
        m_szBaseFile + "Walk/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP_RIGHT),
        m_szBaseFile + "Walk/right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN_LEFT),
        m_szBaseFile + "Walk/left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN_RIGHT),
        m_szBaseFile + "Walk/right/");

    m_pAnimState->SetInitialState(ToString(CharacterState::IDLE_LEFT));
    return m_pAnimState->Initialize();
}

_Use_decl_annotations_
bool pixel_game::EnemyFireDog::InitEnemyAI(_In_ const PG_ENEMY_INIT_DESC& desc)
{
    m_pAIController = std::make_unique<TurretAI>();
    m_pAIController->SetTarget(desc.pTarget);
    m_pAIController->SetAttackDistance(m_nAttackDistance);
    m_pAIController->SetFireCooldown(m_nFireCoolDown);
    m_pAIController->SetMuzzleOffset(m_MuzzleOffset);

    PE_AI_CONTROLLER_DESC controllerDesc{};
    controllerDesc.pAiBody           = m_pBody.get();
    controllerDesc.pAnimStateMachine = m_pAnimState.get();
    m_pAIController->Init(controllerDesc);

    INIT_PROJECTILE_DESC projectileDesc{};
    projectileDesc.OnHit = [&](IProjectile* projectile,
        pixel_engine::BoxCollider* collider)
    {
        if (!collider) return;
        if (!projectile) return;

        ON_PROJECTILE_HIT_EVENT event{};
        event.damage = projectile->GetDamage();
        event.pCollider = collider;
        pixel_engine::EventQueue::Post<ON_PROJECTILE_HIT_EVENT>(event);

        if (projectile)
        {
            projectile->Deactivate();
        }
    };
    projectileDesc.pOwner = m_pAIController->GetBody();

    m_pProjectile = std::make_unique<StraightProjectile>();
    m_pProjectile->Init(projectileDesc);
    m_pProjectile->SetSpeed(m_nProjectileSpeed);
    m_pProjectile->SetLifeSpan(m_nProjectileLifeSpan);
    m_pProjectile->AddHitTag("player");
    m_pProjectile->AddHitTag("Player");

    if (auto* body = m_pProjectile->GetBody())
    {
        body->SetScale({2.5f, 2.5f});

        if (auto* collider = body->GetCollider())
        {
            collider->SetScale({ 2.5f, 2.5f });
        }
    }

    m_pBody->GetCollider()->SetColliderType(pixel_engine::ColliderType::Static);
    
    m_pProjectile->SetDamage(m_nDamage);
    
    auto* anim = m_pProjectile->GetAnimStateMachine();
    anim->AddFrameFromDir("Fire", "assets/ball/fire_ball/");
    anim->SetInitialState("Fire");
    m_pProjectile->GetBody()->SetTexture("assets/ball/fire_ball/1.png");
    if (!m_pProjectile)
    {
        THROW_MSG("Failed to Initialize Projectile!");
    }
    m_pAIController->SetProjectile(m_pProjectile.get());

    return true;
}

void pixel_game::EnemyFireDog::SubscribeEvents()
{
    auto token = pixel_engine::EventQueue::Subscribe<ON_PROJECTILE_HIT_EVENT>
        ([&](const ON_PROJECTILE_HIT_EVENT& event)
            {
                auto* collider = GetCollider();
                if (!collider) return;
                if (event.pCollider != collider) return;

                m_nHealth -= event.damage;

                if (m_nHealth <= 0.f)
                {
                    pixel_engine::logger::debug("fire dog Died!");
                }
            });
    m_tokens.push_back(token);
}

void pixel_game::EnemyFireDog::UnSubscribeEvents()
{
    for (auto& token : m_tokens)
    {
        pixel_engine::EventQueue::Unsubscribe(token);
    }
}

void pixel_game::EnemyFireDog::InitCollisionCallback()
{
    SetOnCollisionEnter();
    SetOnCollisionExit();
}

void pixel_game::EnemyFireDog::AddColliderTags()
{
    if (auto* collider = GetCollider())
    {
        collider->AttachTag("Enemy");
        collider->AttachTag("enemy");
        collider->AttachTag("fire_dog");
        collider->AttachTag("fire_dog");
    }
}

_Use_decl_annotations_
void pixel_game::EnemyFireDog::UpdateAnimState(float deltaTime)
{
    if (!m_pAnimState) return;

    m_pAnimState->OnFrameBegin(deltaTime);
    m_pAnimState->OnFrameEnd();
}

_Use_decl_annotations_
void pixel_game::EnemyFireDog::UpdateAIController(float deltaTime)
{
    if (!m_pAIController) return;

    if (m_pBody && m_pBody->IsVisible())
        m_pAIController->Update(deltaTime);
}

void pixel_game::EnemyFireDog::SetOnCollisionEnter()
{
    auto* collider = GetCollider();
    if (!collider) return;

}

void pixel_game::EnemyFireDog::SetOnCollisionExit()
{
    auto* collider = GetCollider();
    if (!collider) return;

}
