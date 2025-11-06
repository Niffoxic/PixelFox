#include "pch.h"
#include "rigid_body.h"

#include <cmath>
#include <algorithm>


pixel_engine::RigidBody2D::RigidBody2D()
	: m_inverseMass(1.0f)
	, m_linearDamping(0.0f)
	, m_angularDamping(0.0f)
	, m_velocity(0.0f, 0.0f)
	, m_acceleration(0.0f, 0.0f)
	, m_angularVelocity(0.0f)
	, m_forceAcc(0.0f, 0.0f)
	, m_torqueAcc(0.0f)
{
}

void pixel_engine::RigidBody2D::AddForce(const FVector2D& force)
{
	m_forceAcc += force;
}

void pixel_engine::RigidBody2D::AddTorque(float torque)
{
	m_torqueAcc += torque;
}

void pixel_engine::RigidBody2D::Integrate(float deltaTime)
{
	if (deltaTime <= 0.0f)
	{
		ClearAccumulators();
		return;
	}

	FVector2D totalAccel = m_acceleration + m_forceAcc * m_inverseMass;
	m_velocity += totalAccel * deltaTime;

	//~ apply linear damping
	if (m_linearDamping > 0.0f)
	{
		const float damping = std::exp(-m_linearDamping * deltaTime);
		m_velocity *= damping;
	}

	m_transform.Position += m_velocity * deltaTime;

	//~ Angular Motion
	float angularAccel = m_torqueAcc;
	m_angularVelocity += angularAccel * deltaTime;

	if (m_angularDamping > 0.0f)
	{
		const float damping = std::exp(-m_angularDamping * deltaTime);
		m_angularVelocity *= damping;
	}

	m_transform.Rotation += m_angularVelocity * deltaTime;

	ClearAccumulators();
}

//~ getters
FTransform2D pixel_engine::RigidBody2D::GetTransform() const { return m_transform;		  }
FVector2D pixel_engine::RigidBody2D::GetPosition	  () const { return m_transform.Position; }
float pixel_engine::RigidBody2D::GetRotation		  () const { return m_transform.Rotation; }

FVector2D pixel_engine::RigidBody2D::GetVelocity	  () const { return m_velocity;		   }
FVector2D pixel_engine::RigidBody2D::GetAcceleration() const { return m_acceleration;    }
float pixel_engine::RigidBody2D::GetAngularVelocity () const { return m_angularVelocity; }

float pixel_engine::RigidBody2D::GetMass() const
{
	return (m_inverseMass > 0.0f) ? (1.0f / m_inverseMass) : std::numeric_limits<float>::infinity();
}

float pixel_engine::RigidBody2D::GetInverseMass   () const { return m_inverseMass;    }
float pixel_engine::RigidBody2D::GetLinearDamping () const { return m_linearDamping;  }
float pixel_engine::RigidBody2D::GetAngularDamping() const { return m_angularDamping; }

//~ setters
void pixel_engine::RigidBody2D::SetTransform(const FTransform2D& t) { m_transform = t;					  }
void pixel_engine::RigidBody2D::SetPosition (const FVector2D& p)    { m_transform.Position = p;			  }
void pixel_engine::RigidBody2D::SetRotation (float radians)	      { m_transform.Rotation = radians;		  }
void pixel_engine::RigidBody2D::AddRotation (float deltaRadians)    { m_transform.Rotation += deltaRadians; }

void pixel_engine::RigidBody2D::SetVelocity(const FVector2D& velocity) { m_velocity = velocity; }
void pixel_engine::RigidBody2D::AddVelocity(const FVector2D& dv)		 { m_velocity += dv;	  }

void pixel_engine::RigidBody2D::SetAcceleration(const FVector2D& acc) { m_acceleration = acc; }

void pixel_engine::RigidBody2D::SetAngularVelocity(float w)  { m_angularVelocity = w; }
void pixel_engine::RigidBody2D::AddAngularVelocity(float dw) { m_angularVelocity += dw; }

void pixel_engine::RigidBody2D::SetMass(float mass)
{
	if (mass > 0.0f) m_inverseMass = 1.0f / mass;
	else m_inverseMass = 0.0f;
}

void pixel_engine::RigidBody2D::SetInverseMass(float invMass)
{
	m_inverseMass = std::max(0.0f, invMass);
}

void pixel_engine::RigidBody2D::SetLinearDamping(float d)
{
	m_linearDamping = std::max(0.0f, d);
}

void pixel_engine::RigidBody2D::SetAngularDamping(float d)
{
	m_angularDamping = std::max(0.0f, d);
}

void pixel_engine::RigidBody2D::ClearAccumulators()
{
	m_forceAcc = { 0.0f, 0.0f };
	m_torqueAcc = 0.0f;
}
