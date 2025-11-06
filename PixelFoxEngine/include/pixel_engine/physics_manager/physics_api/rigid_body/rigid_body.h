// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once
#include "PixelFoxEngineAPI.h"

#include "fox_math/vector.h"
#include "fox_math/transform.h"

namespace pixel_engine
{
	class PFE_API RigidBody2D
	{
	public:
		RigidBody2D();

		//~ simulate
		void AddForce(const FVector2D& force);
		void AddTorque(float torque);
		void Integrate(float deltaTime);

		//~ Getters
		FTransform2D GetTransform() const;
		FVector2D GetPosition() const;
		float GetRotation() const;

		FVector2D GetVelocity() const;
		FVector2D GetAcceleration() const;
		float GetAngularVelocity() const;

		float GetMass() const;
		float GetInverseMass() const;

		float GetLinearDamping() const;
		float GetAngularDamping() const;

		//~ setters
		void SetTransform(const FTransform2D& t);
		void SetPosition(const FVector2D& p);
		void SetRotation(float radians);
		void AddRotation(float deltaRadians);

		void SetVelocity(const FVector2D& velocity);
		void AddVelocity(const FVector2D& dv);

		void SetAcceleration(const FVector2D& acc);

		void SetAngularVelocity(float w);
		void AddAngularVelocity(float dw);

		void SetMass(float mass);
		void SetInverseMass(float invMass);

		void SetLinearDamping(float d);
		void SetAngularDamping(float d);

		FTransform2D m_transform{};
		FVector2D    m_velocity{};
		FVector2D    m_acceleration{};
		float		 m_angularVelocity{};
		float		 m_inverseMass{ 1.0f };
		float		 m_linearDamping{ 0.25f };
		float		 m_angularDamping{ 0.0f };
		FVector2D	 m_forceAcc{};
		float		 m_torqueAcc{ 0.0f };

	private:
		void ClearAccumulators();

	};
} // namespace fox_physics
