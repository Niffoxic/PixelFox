#include "test_enemy.h"

using namespace pixel_game;

bool TestEnemy::Initialize(const PG_ENEMY_SPAWN& spawnDesc)
{
	if (!InitializeBody(spawnDesc))			return false;
	if (!InitializeAppearance(spawnDesc))	return false;
	if (!InitializeAIController(spawnDesc))	return false;
}

void TestEnemy::Update(float deltaTime)
{
	if (m_pAIController) m_pAIController->Update(deltaTime);
	UpdateAppearance(deltaTime);
}

void TestEnemy::Release()
{
	m_pAIController.reset();
	m_pAIController.reset();
	m_pEnemyBody   .reset();
}

pixel_engine::PEISprite* TestEnemy::GetBody() const
{
	return m_pEnemyBody.get();
}

pixel_engine::AnimSateMachine* TestEnemy::GetAnimState() const
{
	return m_pAnimState.get();
}

IAIController* pixel_game::TestEnemy::GetController() const
{
	return m_pAIController.get();
}

bool TestEnemy::InitializeBody(const PG_ENEMY_SPAWN& spawnDesc)
{
	m_pEnemyBody = std::make_unique<pixel_engine::QuadObject>();
	m_pEnemyBody->SetPosition(
		spawnDesc.SpawnPoint.x,
		spawnDesc.SpawnPoint.y);

	m_pEnemyBody->SetScale(3, 3);
	m_pEnemyBody->SetLayer(pixel_engine::ELayer::Player);
	m_pEnemyBody->GetCollider()->SetColliderType(pixel_engine::ColliderType::Dynamic);
	m_pEnemyBody->GetRigidBody2D()->SetMass(10.f);
	m_pEnemyBody->GetRigidBody2D()->SetLinearDamping(1.f);

	if (!m_pEnemyBody->Initialize()) return false;
	m_pEnemyBody->SetTexture("assets/sprites/player/idle_left/left_0.png");

	return true;
}

bool TestEnemy::InitializeAIController(const PG_ENEMY_SPAWN& spawnDesc)
{
	PE_AI_CONTROLLER_DESC desc{};
	desc.pAiBody		   = m_pEnemyBody.get();
	desc.pAnimStateMachine = m_pAnimState.get();
	m_pAIController = std::make_unique<ChaseAI>();
	m_pAIController->Init(desc);
	m_pAIController->SetTarget(spawnDesc.Target);
	return true;
}

bool TestEnemy::InitializeAppearance(const PG_ENEMY_SPAWN& spawnDesc)
{
	m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pEnemyBody.get());
	return true;
}

void TestEnemy::UpdateAppearance(float deltaTime)
{
	m_pAnimState->OnFrameBegin(deltaTime);
	m_pAnimState->OnFrameEnd();
}
