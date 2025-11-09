#include "pch.h"
#include "straight_projectile.h"

using namespace pixel_game;
using namespace pixel_engine;

_Use_decl_annotations_
bool StraightProjectile::Init(INIT_PROJECTILE_DESC& desc)
{
	m_pOwner = desc.pOwner;
	m_pBody = std::make_unique<QuadObject>();
	if (!m_pBody) return false;

	m_pBody->SetLayer(ELayer::Projectile);
	m_pBody->GetCollider()->SetColliderType(ColliderType::Trigger);

	if (!m_pBody->Initialize()) return false;

	m_pAnimState = std::make_unique<AnimSateMachine>(m_pBody.get());

	m_pBody->SetVisible(false);
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
	if (m_pOwner)
	{
		if (!m_pOwner->IsVisible())
		{
			m_pBody->SetVisible(false);
			Deactivate();
			return;
		}
	}

	if (m_pAnimState)
	{
		m_pAnimState->OnFrameBegin(dt);
		m_pAnimState->OnFrameEnd();
	}
	m_nTimeLeft -= dt;
	if (m_nTimeLeft <= 0.0f)
	{
		if (m_fnOnExpired) m_fnOnExpired(this);
		Deactivate();
		return;
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
	if (m_pOwner)
	{
		if (!m_pOwner->IsVisible())
		{
			m_pBody->SetVisible(false);
			return true;
		}
	}

	const float len2 = directionNorm.LengthSq();
	
	if (len2 > 1e-12f) //~ an epsilon test
	{
		const float inv = 1.0f / std::sqrt(len2);
		m_direction = { directionNorm.x * inv, directionNorm.y * inv };
	}
	else
	{
		m_direction = { 1.f, 0.f }; // fall back to +X to avoid rare cases
	}

	if (speed > 0.f) m_nSpeed = speed;
	else if (m_nSpeed <= 0.f) m_nSpeed = 25.f;

	m_nTimeLeft = m_nLifeSpan;
	m_bActive = true;

	if (auto* rb = m_pBody->GetRigidBody2D())
	{
		rb->SetPosition(worldPos);
		rb->SetRotation(std::atan2(m_direction.y, m_direction.x));
		rb->SetVelocity({ m_direction.x * m_nSpeed, m_direction.y * m_nSpeed });
		rb->SetLinearDamping(0.7f);
	}

	m_pBody->SetVisible(true);
	if (m_fnOnFire) m_fnOnFire(this);
	return true;
}

void StraightProjectile::Deactivate()
{
	if (auto* rb = m_pBody->GetRigidBody2D())
	{
		rb->SetVelocity({ 0.f, 0.f });
		rb->m_transform.Position = { -100000.f, -100000.f };
	}

	m_pBody->SetVisible(false);
	m_bActive   = false;
	m_nTimeLeft = 0.0f;
}

_Use_decl_annotations_
void StraightProjectile::SetActive(bool on)
{
	m_bActive = on;
	if (m_pBody) m_pBody->SetVisible(on);

	if (on && !m_bActive)
	{
		if (m_fnOnActive) m_fnOnActive(this);
	}
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
