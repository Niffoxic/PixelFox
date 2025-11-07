#include "player.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/render_manager/render_queue/render_queue.h"

using namespace pixel_game;


bool PlayerCharacter::Initialize()
{
    pixel_engine::PERenderQueue::Instance().GetCamera()->SetScale({ 1, 1 });

    if (!InitializePlayer    ()) return false;
    if (!InitializeAppearance()) return false;

    m_pBody->GetCollider()->AttachTag("Player");
    m_pBody->GetCollider()->AttachTag("player");
    return true;
}

void PlayerCharacter::Update(float deltaTime)
{
    UpdatePlayerAppearance(deltaTime);
}

void PlayerCharacter::Release()
{

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
    const pixel_engine::PEKeyboardInputs* keyboard,
    float deltaTime)
{
    auto* body = m_pBody->GetRigidBody2D();
    if (!body) return;

    FVector2D direction{ 0.0f, 0.0f };

    if (keyboard->IsKeyPressed('A')) direction.x -= 1.0f; // Left
    if (keyboard->IsKeyPressed('D')) direction.x += 1.0f; // Right
    if (keyboard->IsKeyPressed('W')) direction.y -= 1.0f; // Up
    if (keyboard->IsKeyPressed('S')) direction.y += 1.0f; // Down

    const float lenSq = direction.x * direction.x + direction.y * direction.y;
    if (lenSq > 0.0f)
    {
        const float invLen = 1.0f / std::sqrt(lenSq);
        direction.x *= invLen;
        direction.y *= invLen;
    }

    FVector2D velocity  = direction * m_playerState.movementSpeed * deltaTime;
    body->m_transform.Position += velocity;

    if (keyboard->IsKeyPressed(VK_SPACE))
    {
        // Dash only if not on cooldown
        if (m_playerState.dashCooldownTimer <= 0.0f)
        {
            
            const float dashForce = m_playerState.dashForce;
            body->AddVelocity(direction * dashForce);
            m_playerState.dashCooldownTimer = m_playerState.dashCooldown;
        }
    }

    // Update dash cooldown
    if (m_playerState.dashCooldownTimer > 0.0f)
        m_playerState.dashCooldownTimer -= deltaTime;
}

bool pixel_game::PlayerCharacter::InitializePlayer()
{
    m_pBody = std::make_unique<pixel_engine::QuadObject>();
    m_pBody->SetScale(3, 3);
    m_pBody->SetLayer(pixel_engine::ELayer::Player);
    m_pBody->GetCollider()->SetColliderType(pixel_engine::ColliderType::Dynamic);
    m_pBody->GetRigidBody2D()->SetMass(10.f);
    m_pBody->GetRigidBody2D()->SetLinearDamping(1.f);
    
    if (!m_pBody->Initialize()) return false;
    m_pBody->SetTexture("assets/sprites/player/idle_left/left_0.png");

    pixel_engine::PhysicsQueue::Instance().AddObject(m_pBody.get());
    return true;
}

//~ looks related stuff
bool pixel_game::PlayerCharacter::InitializeAppearance()
{
    m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pBody.get());
    
    m_pAnimState->AddFrameFromDir("Idle_Left", "assets/sprites/player/idle_left/");
    m_pAnimState->AddFrameFromDir("Idle_Right", "assets/sprites/player/idle_right/");
    m_pAnimState->AddFrameFromDir("Dash_Left", "assets/sprites/player/dash_left/");
    m_pAnimState->AddFrameFromDir("Dash_Right", "assets/sprites/player/dash_right/");
    m_pAnimState->AddFrameFromDir("Walk_Right", "assets/sprites/player/walk_right/");
    m_pAnimState->AddFrameFromDir("Walk_Left", "assets/sprites/player/walk_left/");
    
    if (!m_pAnimState->Initialize()) return false;
    m_pAnimState->SetInitialState("Idle_Left");

    return true;
}

void pixel_game::PlayerCharacter::UpdatePlayerAppearance(float deltaTime)
{
    //~ start
    if (m_pAnimState) m_pAnimState->OnFrameBegin(deltaTime);

    //~ end
    if (m_pAnimState) m_pAnimState->OnFrameEnd();
}
