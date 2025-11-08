#include "pch.h"
#include "straight_projectile.h"

using namespace pixel_game;
using namespace pixel_engine;

_Use_decl_annotations_
bool StraightProjectile::Init(INIT_PROJECTILE_DESC& desc)
{
	m_pBody = std::make_unique<QuadObject>();
	if (!m_pBody) return false;

	m_pBody->SetLayer(ELayer::Projectile);
	m_pBody->GetCollider()->SetColliderType(ColliderType::Trigger);

	if (!m_pBody->Initialize()) return false;

	m_pAnimState = std::make_unique<AnimSateMachine>(m_pBody.get());

	PhysicsQueue::Instance().AddObject(m_pBody.get());

	m_bActive  = false;
	m_nTimeLeft = 0.0f;

	//~ set callbacks
	m_fnOnActive  = desc.OnActive;
	m_fnOnExpired = desc.OnExpired;
	m_fnOnFire    = desc.OnFire;
	m_fnOnHit     = desc.OnHit;
	
	return true;
}

_Use_decl_annotations_
void StraightProjectile::Update(float dt)
{
	if (!m_bActive || dt <= 0.f) return;

	m_nTimeLeft -= dt;

	if (m_nTimeLeft <= 0.0f)
	{
		if (m_fnOnExpired) m_fnOnExpired(this);
		Deactivate();
		return;
	}

	auto* rb = m_pBody->GetRigidBody2D();
	if (rb)
	{
		FVector2D pos = rb->GetPosition();
		pos.x += m_direction.x * m_nSpeed * dt;
		pos.y += m_direction.y * m_nSpeed * dt;
		rb->m_transform.Position = pos;
	}
}

_Use_decl_annotations_
bool StraightProjectile::Release()
{
	m_pBody.reset();
	m_bActive = false;
	return true;
}

_Use_decl_annotations_
bool StraightProjectile::Fire(const FVector2D& worldPos,
	const FVector2D& directionNorm,
	float speed)
{
	if (m_bActive) return false;

	m_direction = directionNorm;
	if (speed > 0.f) m_nSpeed = speed;

	m_nTimeLeft = m_nLifeSpan;
	m_bActive = true;
	SetPosition(worldPos);
	SetActive(true);

	if (m_fnOnFire) m_fnOnFire(this);

	return true;
}

void StraightProjectile::Deactivate()
{
	if (!m_bActive) return;

	m_bActive = false;
	SetActive(false);
	m_nTimeLeft = 0.0f;

	if (auto* rb = m_pBody->GetRigidBody2D())
	{
		rb->SetVelocity({ 0.f,0.f });
	}
}

_Use_decl_annotations_
void StraightProjectile::SetActive(bool on)
{
	if (m_pBody) m_pBody->SetVisible(on);

	if (on && !m_bActive)
	{
		if (m_fnOnActive) m_fnOnActive(this);
	}
	
	m_bActive = on;
}

_Use_decl_annotations_
bool StraightProjectile::IsActive() const
{
	return m_bActive; 
}

_Use_decl_annotations_
PEISprite* StraightProjectile::GetBody() const
{
	return m_pBody.get();
}

_Use_decl_annotations_
BoxCollider* StraightProjectile::GetCollider() const
{
	if (m_pBody) return m_pBody->GetCollider();
	return nullptr;
}

_Use_decl_annotations_
void StraightProjectile::SetPosition(const FVector2D& pos)
{
	if (auto* rb = m_pBody->GetRigidBody2D())
		rb->m_transform.Position = pos;
}

_Use_decl_annotations_
FVector2D StraightProjectile::GetPosition() const
{
	if (auto* rb = m_pBody->GetRigidBody2D())
		return rb->GetPosition();

	return { 0,0 };
}

_Use_decl_annotations_
void StraightProjectile::SetDirection(const FVector2D& dirNorm)
{
	m_direction = dirNorm;
}

_Use_decl_annotations_
FVector2D StraightProjectile::GetDirection() const 
{ 
	return m_direction;
}

_Use_decl_annotations_
void StraightProjectile::SetSpeed(float unitsPerSecond)
{
	m_nSpeed = unitsPerSecond;
}

_Use_decl_annotations_
float StraightProjectile::GetSpeed() const
{
	return m_nSpeed; 
}

_Use_decl_annotations_
bool StraightProjectile::SetLifeSpan(float seconds)
{
	m_nLifeSpan = seconds;
	return true;
}

_Use_decl_annotations_
float StraightProjectile::GetLifeSpan() const
{
	return m_nLifeSpan;
}

_Use_decl_annotations_
float StraightProjectile::GetTimeLeft() const
{
	return m_nTimeLeft;
}

_Use_decl_annotations_
void StraightProjectile::SetDamage(float amount)
{
	m_nDamage = amount;
}

_Use_decl_annotations_
float StraightProjectile::GetDamage() const 
{
	return m_nDamage; 
}

void StraightProjectile::OnHit()
{
	if (m_fnOnHit) m_fnOnHit(this);
	Deactivate();
}

pixel_engine::AnimSateMachine* pixel_game::StraightProjectile::GetAnimStateMachine() const
{
	return m_pAnimState.get();
}
