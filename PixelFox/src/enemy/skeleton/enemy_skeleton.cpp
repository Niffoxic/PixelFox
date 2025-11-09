#include "enemy_skeleton.h"

#include "pixel_engine/utilities/logger/logger.h"
#include <filesystem>

#include "world/events/projectile_events.h"
#include "world/events/player_events.h"

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::Initialize(const PG_ENEMY_INIT_DESC& desc)
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
void pixel_game::EnemySkeleton::Update(float deltaTime)
{
    UpdateAnimState(deltaTime);
    UpdateAIController(deltaTime);
}

void pixel_game::EnemySkeleton::Release()
{
    m_bActive = false;
    m_pTarget = nullptr;

    m_pBody.reset();
    m_pAnimState.reset();
    m_pController.reset();
}

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::IsActive() const
{
    return m_bActive;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::EnemySkeleton::GetBody() const
{
    return m_pBody.get();
}

_Use_decl_annotations_
pixel_engine::AnimSateMachine* pixel_game::EnemySkeleton::GetAnimState() const
{
    return m_pAnimState.get();
}

_Use_decl_annotations_
pixel_engine::BoxCollider* pixel_game::EnemySkeleton::GetCollider() const
{
    if (m_pBody && m_pBody->GetCollider()) return m_pBody->GetCollider();
    return nullptr;
}

_Use_decl_annotations_
pixel_game::IAIController* pixel_game::EnemySkeleton::GetController() const
{
    return m_pController.get();
}

_Use_decl_annotations_
void pixel_game::EnemySkeleton::SetTarget(pixel_engine::PEISprite* target)
{
    m_pTarget = target;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::EnemySkeleton::GetTarget() const
{
    return m_pTarget;
}

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::HasTarget() const
{
    return m_pTarget != nullptr;
}

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::IsDead() const
{
    return m_nHealth <= 0.f;
}

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::InitEnemyBody(const PG_ENEMY_INIT_DESC& desc)
{
    m_pBody = std::make_unique<pixel_engine::QuadObject>();
    m_pBody->SetScale(desc.Scale);
    m_pBody->SetPosition(desc.SpawnPoint);
    m_pBody->SetLayer(pixel_engine::ELayer::Npc_AI);
    m_pBody->SetVisible(true);
    m_pBody->GetCollider()->SetColliderType(pixel_engine::ColliderType::Dynamic);

    std::string idle = m_szBaseFile + "Idle/Idle_left/0.png";
    m_pBody->SetTexture(idle);
    return m_pBody->Initialize();
}

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::InitEnemyAnimStateMachine()
{
    m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pBody.get());

    //~ Idle
    ValidatePathExists(m_szBaseFile + "Idle/Idle_left/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_right/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_left/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_right/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_left/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_right/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_left/");
    ValidatePathExists(m_szBaseFile + "Idle/Idle_right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_LEFT), m_szBaseFile + "Idle/Idle_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_RIGHT), m_szBaseFile + "Idle/Idle_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP), m_szBaseFile + "Idle/Idle_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN), m_szBaseFile + "Idle/Idle_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP_LEFT), m_szBaseFile + "Idle/Idle_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP_RIGHT), m_szBaseFile + "Idle/Idle_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN_LEFT), m_szBaseFile + "Idle/Idle_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN_RIGHT), m_szBaseFile + "Idle/Idle_right/");

    //~ Attack
    ValidatePathExists(m_szBaseFile + "Attacking/Attacking_left/");
    ValidatePathExists(m_szBaseFile + "Attacking/Attacking_right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_LEFT), m_szBaseFile + "Attacking/Attacking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_RIGHT), m_szBaseFile + "Attacking/Attacking_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP), m_szBaseFile + "Attacking/Attacking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN), m_szBaseFile + "Attacking/Attacking_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP_LEFT), m_szBaseFile + "Attacking/Attacking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP_RIGHT), m_szBaseFile + "Attacking/Attacking_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN_LEFT), m_szBaseFile + "Attacking/Attacking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN_RIGHT), m_szBaseFile + "Attacking/Attacking_right/");

    //~ Die
    ValidatePathExists(m_szBaseFile + "Dying/Dying_left/");
    ValidatePathExists(m_szBaseFile + "Dying/Dying_right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_LEFT), m_szBaseFile + "Dying/Dying_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_RIGHT), m_szBaseFile + "Dying/Dying_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP), m_szBaseFile + "Dying/Dying_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN), m_szBaseFile + "Dying/Dying_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP_LEFT), m_szBaseFile + "Dying/Dying_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP_RIGHT), m_szBaseFile + "Dying/Dying_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN_LEFT), m_szBaseFile + "Dying/Dying_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN_RIGHT), m_szBaseFile + "Dying/Dying_right/");

    //~ Hurt
    ValidatePathExists(m_szBaseFile + "Hurt/Hurt_left/");
    ValidatePathExists(m_szBaseFile + "Hurt/Hurt_right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_LEFT), m_szBaseFile + "Hurt/Hurt_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_RIGHT), m_szBaseFile + "Hurt/Hurt_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP), m_szBaseFile + "Hurt/Hurt_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN), m_szBaseFile + "Hurt/Hurt_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP_LEFT), m_szBaseFile + "Hurt/Hurt_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP_RIGHT), m_szBaseFile + "Hurt/Hurt_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN_LEFT), m_szBaseFile + "Hurt/Hurt_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN_RIGHT), m_szBaseFile + "Hurt/Hurt_right/");

    //~ Walk
    ValidatePathExists(m_szBaseFile + "Walking/Walking_left/");
    ValidatePathExists(m_szBaseFile + "Walking/Walking_right/");

    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_LEFT), m_szBaseFile + "Walking/Walking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_RIGHT), m_szBaseFile + "Walking/Walking_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP), m_szBaseFile + "Walking/Walking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN), m_szBaseFile + "Walking/Walking_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP_LEFT), m_szBaseFile + "Walking/Walking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP_RIGHT), m_szBaseFile + "Walking/Walking_right/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN_LEFT), m_szBaseFile + "Walking/Walking_left/");
    m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN_RIGHT), m_szBaseFile + "Walking/Walking_right/");

    m_pAnimState->SetInitialState(ToString(CharacterState::WALK_LEFT));
    return m_pAnimState->Initialize();
}

_Use_decl_annotations_
bool pixel_game::EnemySkeleton::InitEnemyAI(_In_ const PG_ENEMY_INIT_DESC& desc)
{
    m_pController = std::make_unique<ChaseAI>();
    m_pController->SetTarget(desc.pTarget);
    m_pController->SetAttackDistance(1.5f);

    PE_AI_CONTROLLER_DESC controllerDesc{};
    controllerDesc.pAiBody = m_pBody.get();
    controllerDesc.pAnimStateMachine = m_pAnimState.get();

    return m_pController->Init(controllerDesc);
}

void pixel_game::EnemySkeleton::SubscribeEvents()
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
                    pixel_engine::logger::debug("Goblin Died!");
                }
            });
    m_tokens.push_back(token);
}

void pixel_game::EnemySkeleton::UnSubscribeEvents()
{
    for (auto& token : m_tokens)
    {
        pixel_engine::EventQueue::Unsubscribe(token);
    }
}

void pixel_game::EnemySkeleton::InitCollisionCallback()
{
    SetOnCollisionEnter();
    SetOnCollisionExit();
}

void pixel_game::EnemySkeleton::AddColliderTags()
{
    if (auto* collider = GetCollider())
    {
        collider->AttachTag("Enemy");
        collider->AttachTag("enemy");
        collider->AttachTag("skeleton");
        collider->AttachTag("Skeleton");
    }
}

_Use_decl_annotations_
void pixel_game::EnemySkeleton::UpdateAnimState(float deltaTime)
{
    if (!m_pAnimState) return;

    m_pAnimState->OnFrameBegin(deltaTime);
    m_pAnimState->OnFrameEnd();
}

_Use_decl_annotations_
void pixel_game::EnemySkeleton::UpdateAIController(float deltaTime)
{
    if (!m_pController) return;

    m_pController->Update(deltaTime);
}

void pixel_game::EnemySkeleton::SetOnCollisionEnter()
{
    auto* collider = GetCollider();
    if (!collider) return;

    //~ Add Specific to player
    collider->SetOnHitEnterCallback([&](pixel_engine::BoxCollider* collider)
        {
            if (!collider) return;

            //~ Player in touch attack him!
            if (collider->HasTag("Player"))
            {
                //~ TODO: Play attack animation
                pixel_engine::logger::debug("Enemy Should Attack Player now");
                m_pAnimState->SetInitialState("attack_left");

                ON_PLAYER_HIT_EVENT event{};
                event.damage = m_nDamage;
                event.hitKnockbackDirection = m_pController->DirectionToTarget();
                event.hitKnockbackPower = m_nKnockBack;

                pixel_engine::EventQueue::Post<ON_PLAYER_HIT_EVENT>(event);

            }

            if (collider->HasTag("Player_Attack"))
            {
                //~ TODO: Play attack animation
                pixel_engine::logger::debug("Enemy Should Get Hit");
            }
        });
}

void pixel_game::EnemySkeleton::SetOnCollisionExit()
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
                pixel_engine::logger::debug("Enemy Should stop Attacking Player now");
            }

            if (collider->HasTag("Player_Attack"))
            {
                //~ TODO: Play attack animation
                pixel_engine::logger::debug("Enemy Should stop Getting Hit");
            }
        });
}
