#pragma once

#include "pixel_engine/PixelEngine.h"

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

		//~ tests
		static constexpr int   kTilePx = 32;
		static constexpr float kUnitW = 1.0f;
		static constexpr float kUnitH = 1.0f;
		static constexpr float kAmpBase = 1.5f;
		static constexpr float kSpeedBase = 4.7f;
		static constexpr float kRotSpeed = 0.75f;

		std::vector<std::unique_ptr<pixel_engine::QuadObject>> m_objects;
		std::vector<fox_math::Vector2D<float>> m_basePos;
		std::vector<float>   m_amp, m_speed, m_phase;
		std::vector<bool>    m_moveX;

		float m_time{ 0.0f };
	};
} // namespace pixel_game
