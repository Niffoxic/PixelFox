#pragma once

#include "PixelFoxEngineAPI.h"
#include "core/vector.h"
#include "pixel_engine/physics_manager/physics_api/collider/contact.h"

namespace pixel_engine
{
	class PFE_API CollisionResolver
	{
	public:
		static void ResolveContact(Contact& contact, float deltaTime);
		static void ResolveContact(fox::vector<Contact>& contacts, float deltaTime);
	private:
		static void ResolveBoxVsBox(Contact& contact, float deltaTime);
		static void ResolvePenetration(Contact& contact, float deltaTime);
	};
} // namespace fox_physics
