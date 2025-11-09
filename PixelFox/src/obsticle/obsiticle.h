#pragma once

#include "fox_math/vector.h"
#include <string>

#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"

namespace pixel_game
{
	typedef struct _INIT_OBSTICLE_DESC
	{
		std::string szName;
		std::string baseTexture;
		std::string obsticleSprites;
		FVector2D   scale;
		FVector2D   position;
	} INIT_OBSTICLE_DESC;

	class Obsticle
	{
	public:
		 Obsticle() = default;
		~Obsticle();
		
		bool Init(const INIT_OBSTICLE_DESC& desc);
		
		void Update(float deltaTime);

		void Draw   ();
		void Hide   ();
		void Release();

		FVector2D	GetPosition() const;
		FVector2D	GetScale   () const;
		std::string GetName	   () const;

		pixel_engine::PEISprite* GetSpirte() const;
		pixel_engine::BoxCollider* GetCollider() const;

	private:
		bool		m_bInit{ false };
		std::string m_szName{ "No Name" };
		
		std::unique_ptr<pixel_engine::QuadObject>	   m_pSprite{ nullptr };
		std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState{ nullptr };
		
		FVector2D m_nPosition{ 0, 0 };
		FVector2D m_nScale	 { 1, 1 };
	};
} // pixel_game
