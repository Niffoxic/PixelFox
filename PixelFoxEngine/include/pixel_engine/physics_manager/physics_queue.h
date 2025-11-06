#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/core/interface/interface_sprite.h"
#include "pixel_engine/render_manager/components/camera/camera.h"

#include "core/unordered_map.h"

namespace pixel_engine
{
	class PFE_API PhysicsQueue final: public ISingleton<PhysicsQueue>
	{
	public:
		PhysicsQueue () = default;
		~PhysicsQueue() = default;

		bool Initialize(Camera2D* camera);
		bool FrameBegin(float delatTime);
		bool FrameEnd ();
		void OnRelease();

		bool AddObject   (PEISprite* sprite);
		bool RemoveObject(PEISprite* sprite);
		bool RemoveObject(UniqueId id);
		void Clear();

	private:
		Camera2D* m_pCamera{ nullptr };
		fox::unordered_map<UniqueId, PEISprite*> m_sprites{};
	};
} // namespace pixel_engine
