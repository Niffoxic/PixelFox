#pragma once

#include "pixel_engine/PixelEngine.h"
#include "world/game_world.h"

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
		std::unique_ptr<GameWorld> m_pGameWorld{ nullptr };
	};
} // namespace pixel_game
