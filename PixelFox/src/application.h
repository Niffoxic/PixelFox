#pragma once

#include "pixel_engine/PixelEngine.h"

namespace pixel_game
{
	class Application final : public pixel_engine::PixelEngine
	{
	public:
		Application() = default;
		~Application();

	protected:
		//~ Pixel Engine Interface
		_NODISCARD _Check_return_ __success(return != false)
		bool InitApplication(
			_In_ const pixel_engine::PIXEL_ENGINE_INIT_DESC& desc
		) override;

		void BeginPlay()				override;
		void Tick	  (float deltaTime) override;
		void Release  ()				override;
	};
} // namespace pixel_game
