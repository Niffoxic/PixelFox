#include "test_turret.h"

using namespace pixel_game;

bool TestEnemyTurrent::Initialize(const PG_ENEMY_SPAWN_TEST& spawnDesc)
{
	if (!InitializeBody(spawnDesc))			return false;
	if (!InitializeAppearance(spawnDesc))	return false;
	if (!InitializeAIController(spawnDesc))	return false;
}

void TestEnemyTurrent::Update(float deltaTime)
{
	if (m_pAIController) m_pAIController->Update(deltaTime);
	UpdateAppearance(deltaTime);
}

void TestEnemyTurrent::Release()
{
	m_pAIController.reset();
	m_pAIController.reset();
	m_pEnemyBody.reset();
}

pixel_engine::PEISprite* TestEnemyTurrent::GetBody() const
{
	return m_pEnemyBody.get();
}

pixel_engine::AnimSateMachine* TestEnemyTurrent::GetAnimState() const
{
	return m_pAnimState.get();
}

IAIController* pixel_game::TestEnemyTurrent::GetController() const
{
	return m_pAIController.get();
}

bool TestEnemyTurrent::InitializeBody(const PG_ENEMY_SPAWN_TEST& spawnDesc)
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

bool TestEnemyTurrent::InitializeAIController(const PG_ENEMY_SPAWN_TEST& spawnDesc)
{
	m_pAIController = std::make_unique<TurretAI>();
	m_pAIController->Init(m_pEnemyBody.get());
	m_pAIController->SetTarget(spawnDesc.Target);
	m_pAIController->SetFireCooldown(0.6f);
	m_pAIController->SetMuzzleOffset({ 0.5f, 0.0f });
	m_pAIController->SetMaxShootDistance(10);

	INIT_PROJECTILE_DESC desc{};
	m_pProjectile = std::make_unique<StraightProjectile>();
	m_pProjectile->Init(desc);
	m_pProjectile->SetSpeed(2.f);
	m_pProjectile->SetLifeSpan(2.f);
	m_pProjectile->SetDamage(1.f);

	m_pAIController->SetProjectile(m_pProjectile.get());

	return true;
}

bool TestEnemyTurrent::InitializeAppearance(const PG_ENEMY_SPAWN_TEST& spawnDesc)
{
	m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pEnemyBody.get());
	return true;
}

void TestEnemyTurrent::UpdateAppearance(float deltaTime)
{
	m_pAnimState->OnFrameBegin(deltaTime);
	m_pAnimState->OnFrameEnd();
}
