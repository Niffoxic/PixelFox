#pragma once

#include "pixel_engine/PixelEngine.h"

#include "player/player.h"
#include "enemy/goblin/enemy_goblin.h"

#include "pixel_engine/render_manager/objects/quad/quad.h"

namespace pixel_game
{
	class Application final : public pixel_engine::PixelEngine
	{
	public:
		Application(_In_opt_ pixel_engine::PIXEL_ENGINE_CONSTRUCT_DESC const* desc);
		~Application();

	protected:
		//~ Pixel Engine Interface
		_NODISCARD _Check_return_ __success(return != false)
		bool InitApplication(
			_In_opt_ pixel_engine::PIXEL_ENGINE_INIT_DESC const* desc
		) override;

		void BeginPlay()				override;
		void Tick	  (float deltaTime) override;
		void Release  ()				override;

	private:
		PlayerCharacter  m_player{};
		EnemyGoblin		 m_enemy {};

		pixel_engine::Camera2D* m_pCamera2D{ nullptr };

		std::unique_ptr<pixel_engine::QuadObject> m_object{ nullptr };
		std::unique_ptr<pixel_engine::QuadObject> m_object_1{ nullptr };
		
	};
} // namespace pixel_game
