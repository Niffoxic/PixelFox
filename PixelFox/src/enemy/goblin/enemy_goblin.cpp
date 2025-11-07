#include "enemy_goblin.h"

#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "world/events/projectile_events.h"

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::Initialize(const PG_ENEMY_INIT_DESC& desc)
{
    SetTarget(desc.pTarget);

    if (!InitEnemyBody(desc)) return false;
    if (!InitEnemyAnimStateMachine()) return false;
    if (!InitEnemyAI(desc)) return false;

    SubscribeEvents      ();
    InitCollisionCallback();
    AddColliderTags      ();

    pixel_engine::PhysicsQueue::Instance().AddObject(m_pBody.get());

    return true;
}

_Use_decl_annotations_
void pixel_game::EnemyGoblin::Update(float deltaTime)
{
    UpdateAnimState   (deltaTime);
    UpdateAIController(deltaTime);
}

void pixel_game::EnemyGoblin::Release()
{
    m_bActive = false;
    m_pTarget = nullptr;

    if (m_pBody)
    {
        pixel_engine::PhysicsQueue::Instance().RemoveObject(m_pBody.get());
    }

    m_pBody      .reset();
    m_pAnimState .reset();
    m_pController.reset();
}

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::IsActive() const
{
    return m_bActive;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::EnemyGoblin::GetBody() const
{
    return m_pBody.get();
}

_Use_decl_annotations_
pixel_engine::AnimSateMachine* pixel_game::EnemyGoblin::GetAnimState() const
{
    return m_pAnimState.get();
}

_Use_decl_annotations_
pixel_engine::BoxCollider* pixel_game::EnemyGoblin::GetCollider() const
{
    if (m_pBody && m_pBody->GetCollider()) return m_pBody->GetCollider();
    return nullptr;
}

_Use_decl_annotations_
pixel_game::IAIController* pixel_game::EnemyGoblin::GetController() const
{
    return m_pController.get();
}

_Use_decl_annotations_
void pixel_game::EnemyGoblin::SetTarget(pixel_engine::PEISprite* target)
{
    m_pTarget = target;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::EnemyGoblin::GetTarget() const
{
    return m_pTarget;
}

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::HasTarget() const
{
    return m_pTarget != nullptr;
}

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::IsDead() const
{
    return m_nHealth <= 0.f;
}

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::InitEnemyBody(const PG_ENEMY_INIT_DESC& desc)
{
    m_pBody = std::make_unique<pixel_engine::QuadObject>();
    m_pBody->SetScale(desc.Scale);
    m_pBody->SetPosition(desc.SpawnPoint);
    m_pBody->SetLayer(pixel_engine::ELayer::Npc_AI);
    m_pBody->SetVisible(true);

    std::string idle = m_szBaseFile + "Idle/idle_left/00.png";
    m_pBody->SetTexture(idle);
    return m_pBody->Initialize();
}

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::InitEnemyAnimStateMachine()
{
    m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pBody.get());
    
    m_pAnimState->AddFrameFromDir("idle_left", m_szBaseFile + "Idle/idle_left/");
    m_pAnimState->AddFrameFromDir("idle_right", m_szBaseFile + "Idle/idle_right/");
    m_pAnimState->AddFrameFromDir("attack_left", m_szBaseFile + "Attacking/attacking_left/");
    m_pAnimState->AddFrameFromDir("attack_right", m_szBaseFile + "Attacking/attacking_right/");
    m_pAnimState->AddFrameFromDir("die_left", m_szBaseFile + "Dying/dying_left/");
    m_pAnimState->AddFrameFromDir("die_right", m_szBaseFile + "Dying/dying_right/");
    m_pAnimState->AddFrameFromDir("hurt_left", m_szBaseFile + "Hurt/hurt_left/");
    m_pAnimState->AddFrameFromDir("hurt_right", m_szBaseFile + "Hurt/hurt_right/");
    m_pAnimState->AddFrameFromDir("walk_left", m_szBaseFile + "Walking/walking_left/");
    m_pAnimState->AddFrameFromDir("walk_right", m_szBaseFile + "Walking/walking_right/");
    
    m_pAnimState->SetInitialState("walk_left");

    return m_pAnimState->Initialize();
}

_Use_decl_annotations_
bool pixel_game::EnemyGoblin::InitEnemyAI(_In_ const PG_ENEMY_INIT_DESC& desc)
{
    m_pController = std::make_unique<ChaseAI>();
    m_pController->SetTarget(desc.pTarget);
    
    return m_pController->Init(m_pBody.get());
}

void pixel_game::EnemyGoblin::SubscribeEvents()
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
                    // TODO: Call Death
                    pixel_engine::logger::debug("Goblin Died!");
                }
            });
    m_tokens.push_back(token);
}

void pixel_game::EnemyGoblin::UnSubscribeEvents()
{
    for (auto& token : m_tokens)
    {
        pixel_engine::EventQueue::Unsubscribe(token);
    }
}

void pixel_game::EnemyGoblin::InitCollisionCallback()
{
    SetOnCollisionEnter();
    SetOnCollisionExit ();
}

void pixel_game::EnemyGoblin::AddColliderTags()
{
    if (auto* collider = GetCollider())
    {
        collider->AttachTag("Enemy");
        collider->AttachTag("enemy");
        collider->AttachTag("Goblin");
        collider->AttachTag("goblin");
    }
}

_Use_decl_annotations_
void pixel_game::EnemyGoblin::UpdateAnimState(float deltaTime)
{
    if (!m_pAnimState) return;

    m_pAnimState->OnFrameBegin(deltaTime);
    m_pAnimState->OnFrameEnd();
}

_Use_decl_annotations_
void pixel_game::EnemyGoblin::UpdateAIController(float deltaTime)
{
    if (!m_pController) return;

    m_pController->Update(deltaTime);
}

void pixel_game::EnemyGoblin::SetOnCollisionEnter()
{
    auto* collider = GetCollider();
    if (!collider) return;

    //~ Add Specific to player
    collider->SetOnHitEnterCallback([&](pixel_engine::BoxCollider* collider)
        {
            if (!collider) return;

            if (collider->HasTag("Player"))
            {
                //~ TODO: Play attack animation
                pixel_engine::logger::debug("Enemy Should Attack Player now");
                m_pAnimState->SetInitialState("attack_left");
            }

            if (collider->HasTag("Player_Attack"))
            {
                //~ TODO: Play attack animation
                pixel_engine::logger::debug("Enemy Should Get Hit");
            }
        });
}

void pixel_game::EnemyGoblin::SetOnCollisionExit()
{
    auto* collider = GetCollider();
    if (!collider) return;

    //~ Add Specific to player
    collider->SetOnHitExitCallback([&](pixel_engine::BoxCollider* collider)
        {
            if (!collider) return;

            if (collider->HasTag("Player"))
            {
                //~ TODO: Play attack animation
                m_pAnimState->SetInitialState("walk_left");
                pixel_engine::logger::debug("Enemy Should stop Attacking Player now");
            }

            if (collider->HasTag("Player_Attack"))
            {
                //~ TODO: Play attack animation
                pixel_engine::logger::debug("Enemy Should stop Getting Hit");
            }
        });
}
