#include "player.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/render_manager/render_queue/render_queue.h"

#include "world/state/character_state.h"

//~ states
#include "player_state/idle/idle_state.h"
#include "player_state/move/move_state.h"
#include "player_state/dash/dash_state.h"

//~ events
#include "pixel_engine/core/event/event_queue.h"
#include "world/events/projectile_events.h"

using namespace pixel_game;

static pixel_game::IPlayerState* GetState(pixel_game::EPlayerStateId id)
{
    using namespace pixel_game;
    static PlayerStateIdle sIdle;
    static PlayerStateMove sMove;
    static PlayerStateDash sDash;
    switch (id)
    {
    case EPlayerStateId::Idle: return &sIdle;
    case EPlayerStateId::Move: return &sMove;
    case EPlayerStateId::Dash: return &sDash;
    default:                   return &sIdle;
    }
}

bool PlayerCharacter::Initialize()
{
    if (m_bInitialized) return true;
    m_bInitialized = true;
    pixel_engine::PERenderQueue::Instance().GetCamera()->SetScale({ 1, 1 });

    if (!InitializePlayer    ()) return false;
    if (!InitializeAppearance()) return false;

    m_pBody->GetCollider()->AttachTag("Player");
    m_pBody->GetCollider()->AttachTag("player");

    m_pBasicAttack = std::make_unique<StraightProjectile>();
    INIT_PROJECTILE_DESC desc{};
    desc.pOwner = m_pBody.get();
    desc.OnHit =
        [&](IProjectile* projectile, pixel_engine::BoxCollider* collider)
        {
            if (!collider) return;
            
            ON_PROJECTILE_HIT_EVENT event{};
            event.damage    = m_nProjectileDamage;
            event.pCollider = collider;
            pixel_engine::EventQueue::Post<ON_PROJECTILE_HIT_EVENT>(event);

            if (projectile)
            {
                projectile->Deactivate();
            }
        };

    m_pBasicAttack->Init(desc);

    m_pBasicAttack->GetBody()->SetTexture("assets/ball/fire_ball/1.png");
    m_pBasicAttack->SetActive(false);
    m_pBasicAttack->SetDamage   (m_nProjectileDamage);
    m_pBasicAttack->SetLifeSpan (m_nProjectileLifeSpan);
    m_pBasicAttack->SetSpeed    (m_nProjectileSpeed);

    m_pBasicAttack->AddHitTag("Enemy");
    m_pBasicAttack->AddHitTag("enemy");

    m_pSpecialAttack = std::make_unique<StraightProjectile>();
    desc.OnHit =
        [&](IProjectile* projectile, pixel_engine::BoxCollider* collider)
        {
            if (!collider) return;
            if (collider == m_pBody->GetCollider()) return;
            if (!collider->HasTag("Enemy")) return;
            m_aoeVictims[collider] = true;
        };


    m_pSpecialAttack->Init(desc);
    m_pSpecialAttack->GetBody()->SetLayer(pixel_engine::ELayer::Obstacles);
    m_pSpecialAttack->SetLifeSpan(m_nSpclLifeSpan);
    m_pSpecialAttack->SetDamage(m_nSpclDamange);
    m_pSpecialAttack->GetBody()->SetScale({ 10, 10 });
    m_pSpecialAttack->GetBody()->GetCollider()->SetScale({ 10, 10 });
    m_pSpecialAttack->GetBody()->SetTexture("assets/ball/fire_ball/1.png");
    m_pSpecialAttack->SetSpeed(0.2f);
    m_pSpecialAttack->AddHitTag("Enemy");
    m_pSpecialAttack->AddHitTag("enemy");

    return true;
}

void PlayerCharacter::Update(float deltaTime)
{
    UpdatePlayerState(deltaTime);

    UpdatePlayerAppearance(deltaTime);
    Attack                (deltaTime);
    AttackSpecial         (deltaTime);
}

void PlayerCharacter::Release()
{

}

void pixel_game::PlayerCharacter::Draw()
{
    if (!m_bInitialized) return;
    m_pBody->SetVisible(true);
}

void pixel_game::PlayerCharacter::Hide()
{
    if (!m_bInitialized) return;
    m_pBody->SetVisible(false);
}

void pixel_game::PlayerCharacter::UnloadFromQueue()
{
    if (!m_bInitialized) return;
    pixel_engine::PhysicsQueue::Instance().RemoveObject(m_pBody.get());
}

pixel_engine::PEISprite* PlayerCharacter::GetPlayerBody() const
{
    return m_pBody.get();
}

pixel_engine::AnimSateMachine* PlayerCharacter::GetPlayerAnimState() const
{
    return m_pAnimState.get();
}

void pixel_game::PlayerCharacter::HandleInput(
    pixel_engine::PEKeyboardInputs* keyboard,
    float deltaTime)
{
    m_pKeyboard = keyboard;

    FVector2D dir{ 0.f, 0.f };
    if (keyboard->IsKeyPressed('A')) dir.x -= 1.f;
    if (keyboard->IsKeyPressed('D')) dir.x += 1.f;
    if (keyboard->IsKeyPressed('W')) dir.y -= 1.f;
    if (keyboard->IsKeyPressed('S')) dir.y += 1.f;
    
    if (keyboard->IsKeyPressed('E'))
    {
        if (m_nSpclFireCoolDownTimer > 0.f) return;
        m_nSpclFireCoolDownTimer = m_nSpclFireCoolDown;
        m_bSpclLaunched = true;
    }

    const float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq > 0.f) {
        const float invLen = 1.f / std::sqrt(lenSq);
        dir.x *= invLen; dir.y *= invLen;
    }
    m_direction = dir;
    if (dir.x != 0.f || dir.y != 0.f) m_lastNonZeroDir = dir;
}

bool pixel_game::PlayerCharacter::InitializePlayer()
{
    m_pBody = std::make_unique<pixel_engine::QuadObject>();
    m_pBody->SetScale(5, 5);
    m_pBody->SetLayer(pixel_engine::ELayer::Player);
    m_pBody->GetCollider()->SetColliderType(pixel_engine::ColliderType::Dynamic);
    m_pBody->GetRigidBody2D()->SetMass(10.f);
    m_pBody->GetRigidBody2D()->SetLinearDamping(1.f);
    
    if (!m_pBody->Initialize()) return false;
    m_pBody->SetTexture("assets/player/base.png");

    m_pBody->SetVisible(false);
    pixel_engine::PhysicsQueue::Instance().AddObject(m_pBody.get());
    return true;
}

//~ looks related stuff
bool pixel_game::PlayerCharacter::InitializeAppearance()
{
    m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pBody.get());
    if (!m_pAnimState) return false;

    std::string path;

    //~ attack-1
    path = m_szPlayerBase + "attack/right/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_RIGHT), path);

    path = m_szPlayerBase + "attack/right_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP_RIGHT), path);

    path = m_szPlayerBase + "attack/up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP), path);
    
    path = m_szPlayerBase + "attack/left_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_UP_LEFT), path);
    
    path = m_szPlayerBase + "attack/left/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_LEFT), path);
    
    path = m_szPlayerBase + "attack/left_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN_LEFT), path);
    
    path = m_szPlayerBase + "attack/down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN), path);
    
    path = m_szPlayerBase + "attack/right_down/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK1_DOWN_RIGHT), path);

    //~ attack-2
    path = m_szPlayerBase + "special_attack/right/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_RIGHT), path);
    
    path = m_szPlayerBase + "special_attack/right_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_UP_RIGHT), path);
    
    
    path = m_szPlayerBase + "special_attack/up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_UP), path);
    
    path = m_szPlayerBase + "special_attack/left_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_UP_LEFT), path);
    
    path = m_szPlayerBase + "special_attack/left/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_LEFT), path);
    
    path = m_szPlayerBase + "special_attack/left_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_DOWN_LEFT), path);
    
    path = m_szPlayerBase + "special_attack/down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_DOWN), path);
   
    path = m_szPlayerBase + "special_attack/right_down/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::ATTACK2_DOWN_RIGHT), path);

    //~ dash
    path = m_szPlayerBase + "dash/right/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_RIGHT), path);
        auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_RIGHT));
        tile->EnableLoop(false);
    }    

    path = m_szPlayerBase + "dash/right_up/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_UP_RIGHT), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_UP_RIGHT)))
            tile->EnableLoop(false);
    }

    path = m_szPlayerBase + "dash/up/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_UP), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_UP)))
            tile->EnableLoop(false);
    }

    path = m_szPlayerBase + "dash/left_up/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_UP_LEFT), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_UP_LEFT)))
            tile->EnableLoop(false);
    }

    path = m_szPlayerBase + "dash/left/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_LEFT), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_LEFT)))
            tile->EnableLoop(false);
    }

    path = m_szPlayerBase + "dash/left_down/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_DOWN_LEFT), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_DOWN_LEFT)))
            tile->EnableLoop(false);
    }

    path = m_szPlayerBase + "dash/down/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_DOWN), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_DOWN)))
            tile->EnableLoop(false);
    }

    path = m_szPlayerBase + "dash/right_down/";
    if (ValidatePathExists(path))
    {
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DASH_DOWN_RIGHT), path);
        if (auto* tile = m_pAnimState->GetTileAnim(ToString(CharacterState::DASH_DOWN_RIGHT)))
            tile->EnableLoop(false);
    }

    //~ idle
    path = m_szPlayerBase + "idle/right/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_RIGHT), path);
    
    path = m_szPlayerBase + "idle/right_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP_RIGHT), path);
    
    path = m_szPlayerBase + "idle/up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP), path);
    
    path = m_szPlayerBase + "idle/left_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_UP_LEFT), path);
    
    path = m_szPlayerBase + "idle/left/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_LEFT), path);
    
    path = m_szPlayerBase + "idle/left_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN_LEFT), path);
    
    path = m_szPlayerBase + "idle/down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN), path);
    
    path = m_szPlayerBase + "idle/right_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::IDLE_DOWN_RIGHT), path);

    //~ walk
    path = m_szPlayerBase + "walk/right/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_RIGHT), path);
    
    path = m_szPlayerBase + "walk/right_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP_RIGHT), path);
    
    path = m_szPlayerBase + "walk/up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP), path);
    
    path = m_szPlayerBase + "walk/left_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_UP_LEFT), path);
    
    path = m_szPlayerBase + "walk/left/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_LEFT), path);
    
    path = m_szPlayerBase + "walk/left_down/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN_LEFT), path);
    
    path = m_szPlayerBase + "walk/down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN), path);
    
    path = m_szPlayerBase + "walk/right_down/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::WALK_DOWN_RIGHT), path);

    //~ hurt
    path = m_szPlayerBase + "hurt/right/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_RIGHT), path);
    path = m_szPlayerBase + "hurt/right_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP_RIGHT), path);
    path = m_szPlayerBase + "hurt/up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP), path);
    path = m_szPlayerBase + "hurt/left_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_UP_LEFT), path);
    path = m_szPlayerBase + "hurt/left/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_LEFT), path);
    path = m_szPlayerBase + "hurt/left_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN_LEFT), path);
    path = m_szPlayerBase + "hurt/down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN), path);
    path = m_szPlayerBase + "hurt/right_down/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::HURT_DOWN_RIGHT), path);

    //~ death
    path = m_szPlayerBase + "death/right/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_RIGHT), path);
    
    path = m_szPlayerBase + "death/right_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP_RIGHT), path);
    
    path = m_szPlayerBase + "death/up/";
    if (ValidatePathExists(path)) 
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP), path);
    
    path = m_szPlayerBase + "death/left_up/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_UP_LEFT), path);
    
    path = m_szPlayerBase + "death/left/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_LEFT), path);
    
    path = m_szPlayerBase + "death/left_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN_LEFT), path);
    
    path = m_szPlayerBase + "death/down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN), path);
    
    path = m_szPlayerBase + "death/right_down/";
    if (ValidatePathExists(path))
        m_pAnimState->AddFrameFromDir(ToString(CharacterState::DIE_DOWN_RIGHT), path);

    if (!m_pAnimState->Initialize()) return false;
    m_pAnimState->SetInitialState(ToString(CharacterState::WALK_LEFT));

    return true;
}

void pixel_game::PlayerCharacter::UpdatePlayerAppearance(float deltaTime)
{
    //~ start
    if (m_pAnimState) m_pAnimState->OnFrameBegin(deltaTime);

    //~ end
    if (m_pAnimState) m_pAnimState->OnFrameEnd();
}

void pixel_game::PlayerCharacter::UpdatePlayerState(float deltaTime)
{
    if (!m_pAnimState) return;

    if (!m_pState)
    {
        m_eState = EPlayerStateId::Idle;
        m_pState = GetState(m_eState);

        PlayerContext ctx
        {
            this, m_pAnimState.get(),
            m_pKeyboard,
            deltaTime,
            m_playerState.movementSpeed,
            m_playerState.dashForce,
            m_playerState.dashCooldown,
            m_playerState.dashCooldownTimer,
            m_nDashDuration,
            m_nDashTimer,
            m_nSpclFireCoolDown,
            m_nSpclFireCoolDownTimer,
            m_direction,
            m_lastNonZeroDir
        };
        m_pState->OnEnter(ctx);
    }

    PlayerContext ctx
    {
        this, m_pAnimState.get(),
        m_pKeyboard,
        deltaTime,
        m_playerState.movementSpeed,
        m_playerState.dashForce,
        m_playerState.dashCooldown,
        m_playerState.dashCooldownTimer,
        m_nDashDuration,
        m_nDashTimer,
        m_nSpclFireCoolDown,
        m_nSpclFireCoolDownTimer,
        m_direction,
        m_lastNonZeroDir
    };

    const EPlayerStateId next = m_pState->Tick(ctx);
    if (next != m_eState)
    {
        m_pState->OnExit(ctx);
        m_eState = next;
        m_pState = GetState(m_eState);
        m_pState->OnEnter(ctx);
    }

    if (m_playerState.dashCooldownTimer > 0.f)
        m_playerState.dashCooldownTimer -= deltaTime;
    if (m_nSpclFireCoolDownTimer > 0.f)
        m_nSpclFireCoolDownTimer -= deltaTime;
}

void pixel_game::PlayerCharacter::Attack(float deltaTime)
{
    if (!m_pBasicAttack) return;
    m_pBasicAttack->Update(deltaTime);
    
    static float fireTimer = 0.f;
    if (fireTimer > 0.f)
    {
        fireTimer -= deltaTime;
        return;
    }

    auto* body = m_pBody->GetRigidBody2D();
    if (!body) return;

    const FVector2D playerPos = body->GetPosition();
    const FVector2D targetPos = m_nearestLoc;
    
    FVector2D toTarget = targetPos - playerPos;

    const float distSq = toTarget.LengthSq();
    if (distSq > (m_nAttackDistance * m_nAttackDistance))
        return;
    
    const float len = std::sqrt(distSq);
    if (len > 0.f)
    {
        toTarget.x /= len;
        toTarget.y /= len;
    }
    else
    {
        toTarget = { 1.f, 0.f };
    }

    FVector2D muzzlePos = playerPos;

    // m_pBasicAttack->Fire(muzzlePos, toTarget, m_nProjectileSpeed);
    fireTimer = m_nFireCoolDown;
}

void pixel_game::PlayerCharacter::AttackSpecial(float deltaTime)
{
    //~ tick AoE cadence timer
    if (m_nNextDmgCoolDownTimer > 0.f)
    {
        m_nNextDmgCoolDownTimer -= deltaTime;
        if (m_nNextDmgCoolDownTimer < 0.f) m_nNextDmgCoolDownTimer = 0.f;
    }

    if (!m_pSpecialAttack) return;
    m_pSpecialAttack->Update(deltaTime);

    //~ launch when requested and allowed
    if (m_bSpclLaunched && m_bCanLaunch)
    {
        m_bSpclLaunched = false;
        m_bCanLaunch = false;

        auto* rb = m_pBody->GetRigidBody2D();
        if (!rb) return;

        const FVector2D playerPos = rb->GetPosition();
        const FVector2D densePos = m_bestLoc;

        FVector2D dir = densePos - playerPos;
        const float len2 = dir.LengthSq();
        if (len2 > 1e-6f)
        {
            const float inv = 1.f / std::sqrt(len2);
            dir.x *= inv; dir.y *= inv;
        }
        else
        {
            dir = { 1.f, 0.f };
        }

        m_pSpecialAttack->SetDamage(m_nSpclDamange);
        m_pSpecialAttack->SetLifeSpan(m_nSpclLifeSpan);

        const float almostStatic = 0.001f;
        m_pSpecialAttack->Fire(densePos, dir, almostStatic);

        //~ first AoE tick immediately on launch
        m_nNextDmgCoolDownTimer = 0.f;
    }

    //~ AoE tick while active
    if (m_pSpecialAttack->IsActive())
    {
        if (m_nNextDmgCoolDownTimer <= 0.f)
        {
            for (const auto& kv : m_aoeVictims)
            {
                auto* col = kv.first;
                if (!col) continue;

                ON_PROJECTILE_HIT_EVENT ev{};
                ev.damage = m_nSpclDamange;
                ev.pCollider = col;
                pixel_engine::EventQueue::Post<ON_PROJECTILE_HIT_EVENT>(ev);
            }
            m_nNextDmgCoolDownTimer = m_nNextDmgCoolDown;
        }
    }
    else
    {
        //~ start cooldown after projectile ends
        if (!m_bCanLaunch && m_nSpclFireCoolDownTimer <= 0.f)
            m_nSpclFireCoolDownTimer = m_nSpclFireCoolDown;

        if (m_nSpclFireCoolDownTimer > 0.f)
        {
            m_nSpclFireCoolDownTimer -= deltaTime;
            if (m_nSpclFireCoolDownTimer <= 0.f)
            {
                m_nSpclFireCoolDownTimer = 0.f;
                m_bCanLaunch = true;
            }
        }

        m_aoeVictims.clear();
        m_nNextDmgCoolDownTimer = 0.f;
    }
}
