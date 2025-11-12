#include "enemy_lord.h"

#include "pixel_engine/utilities/logger/logger.h"

#include "world/events/projectile_events.h"
#include "world/events/player_events.h"

_Use_decl_annotations_
bool pixel_game::DemonLord::Initialize(const PG_ENEMY_INIT_DESC& desc)
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
void pixel_game::DemonLord::Update(float deltaTime)
{
    UpdateAnimState(deltaTime);
    UpdateAIController(deltaTime);

    if (!m_bShowingHP && m_pBody->IsVisible())
    {
        m_bShowingHP = true;
        pixel_engine::PERenderQueue::Instance().AddFont(m_pFont.get());
    }

    UpdateHealthBar();
}

void pixel_game::DemonLord::Release()
{
    m_bActive = false;
    m_pTarget = nullptr;

    m_pBody.reset();
    m_pAnimState.reset();
    m_pController.reset();
}

_Use_decl_annotations_
bool pixel_game::DemonLord::IsActive() const
{
    return m_bActive;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::DemonLord::GetBody() const
{
    return m_pBody.get();
}

_Use_decl_annotations_
pixel_engine::AnimSateMachine* pixel_game::DemonLord::GetAnimState() const
{
    return m_pAnimState.get();
}

_Use_decl_annotations_
pixel_engine::BoxCollider* pixel_game::DemonLord::GetCollider() const
{
    if (m_pBody && m_pBody->GetCollider()) return m_pBody->GetCollider();
    return nullptr;
}

_Use_decl_annotations_
pixel_game::IAIController* pixel_game::DemonLord::GetController() const
{
    return m_pController.get();
}

_Use_decl_annotations_
void pixel_game::DemonLord::SetTarget(pixel_engine::PEISprite* target)
{
    m_pTarget = target;
}

_Use_decl_annotations_
pixel_engine::PEISprite* pixel_game::DemonLord::GetTarget() const
{
    return m_pTarget;
}

_Use_decl_annotations_
bool pixel_game::DemonLord::HasTarget() const
{
    return m_pTarget != nullptr;
}

_Use_decl_annotations_
bool pixel_game::DemonLord::IsDead() const
{
    return m_nHealth <= 0.f;
}

_Use_decl_annotations_
bool pixel_game::DemonLord::InitEnemyBody(const PG_ENEMY_INIT_DESC& desc)
{
    m_pBody = std::make_unique<pixel_engine::QuadObject>();
    auto scale = m_scale;
    scale.x += 7;
    scale.y += 3;
    m_pBody->SetScale(scale);
    m_pBody->SetPosition(desc.SpawnPoint);
    m_pBody->SetLayer(pixel_engine::ELayer::Npc_AI);
    m_pBody->SetVisible(true);
    m_pBody->GetCollider()->SetColliderType(
        pixel_engine::ColliderType::Dynamic);

    scale = m_scale;
    scale.y = 3;
    scale.x = 3;
    m_pBody->GetCollider()->SetScale(scale);

    std::string idle = m_szBaseFile + "Idle/left/0.png";
    m_pBody->SetTexture(idle);

    m_pFont = std::make_unique<pixel_engine::PEFont>();
    m_pFont->SetPx(32);
    m_pFont->SetPosition({ 900, 50 });
    m_pFont->SetText("Boss HP: ");

    return m_pBody->Initialize();
}

_Use_decl_annotations_
bool pixel_game::DemonLord::InitEnemyAnimStateMachine()
{
    m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pBody.get());

#define ADD_8DIR_BACK_FRONT(BASEFOLDER, STATEPREFIX)\
        do {                                        \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_LEFT),       m_szBaseFile + BASEFOLDER "left/");  \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_RIGHT),      m_szBaseFile + BASEFOLDER "right/"); \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_UP),         m_szBaseFile + BASEFOLDER "left/");  \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_DOWN),       m_szBaseFile + BASEFOLDER "right/"); \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_UP_LEFT),    m_szBaseFile + BASEFOLDER "left/");  \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_UP_RIGHT),   m_szBaseFile + BASEFOLDER "right/");  \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_DOWN_LEFT),  m_szBaseFile + BASEFOLDER "left/"); \
            m_pAnimState->AddFrameFromDir(ToString(CharacterState::STATEPREFIX##_DOWN_RIGHT), m_szBaseFile + BASEFOLDER "right/"); \
        } while (0)

    ADD_8DIR_BACK_FRONT("Idle/",      IDLE);
    ADD_8DIR_BACK_FRONT("Attacking/", ATTACK1);
    ADD_8DIR_BACK_FRONT("Dying/",     DIE);
    ADD_8DIR_BACK_FRONT("Hurt/",      HURT);
    ADD_8DIR_BACK_FRONT("Walking/",   WALK);

    m_pAnimState->SetInitialState(ToString(CharacterState::WALK_LEFT));

#undef ADD_8DIR_BACK_FRONT
    return m_pAnimState->Initialize();
}

_Use_decl_annotations_
bool pixel_game::DemonLord::InitEnemyAI(_In_ const PG_ENEMY_INIT_DESC& desc)
{
    m_pController = std::make_unique<ChaseAI>();
    m_pController->SetTarget(desc.pTarget);
    m_pController->SetAttackDistance(10.f);

    PE_AI_CONTROLLER_DESC controllerDesc{};
    controllerDesc.pAiBody = m_pBody.get();
    controllerDesc.pAnimStateMachine = m_pAnimState.get();

    return m_pController->Init(controllerDesc);
}

void pixel_game::DemonLord::SubscribeEvents()
{
    auto token = pixel_engine::EventQueue::Subscribe<ON_PROJECTILE_HIT_EVENT>
        ([&](const ON_PROJECTILE_HIT_EVENT& event)
            {
                auto* collider = GetCollider();
                if (!collider) return;
                if (event.pCollider != collider) return;

                pixel_engine::logger::error(
                    "I'm taking hit!! from {} to {}",
                    m_nHealth, m_nHealth - event.damage);

                m_nHealth -= event.damage;

                if (m_nHealth <= 0.f)
                {
                    pixel_engine::logger::debug("Goblin Died!");
                }
            });
    m_tokens.push_back(token);
}

void pixel_game::DemonLord::UnSubscribeEvents()
{
    for (auto& token : m_tokens)
    {
        pixel_engine::EventQueue::Unsubscribe(token);
    }
}

void pixel_game::DemonLord::InitCollisionCallback()
{
    SetOnCollisionEnter();
    SetOnCollisionExit();
}

void pixel_game::DemonLord::AddColliderTags()
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
void pixel_game::DemonLord::UpdateAnimState(float deltaTime)
{
    if (!m_pAnimState) return;

    m_pAnimState->OnFrameBegin(deltaTime);
    m_pAnimState->OnFrameEnd();
}

_Use_decl_annotations_
void pixel_game::DemonLord::UpdateAIController(float deltaTime)
{
    if (!m_pController) return;

    m_pController->Update(deltaTime);
}

void pixel_game::DemonLord::UpdateHealthBar()
{
    if (m_pFont && m_bShowingHP) 
    {
        int hp = m_nHealth;
        std::string message = "Boss HP: " + std::to_string(hp);
        m_pFont->SetText(message);
    }
}

void pixel_game::DemonLord::SetOnCollisionEnter()
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

void pixel_game::DemonLord::SetOnCollisionExit()
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
