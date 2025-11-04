#pragma once

#include "PixelFoxEngineAPI.h"

#include "fox_math/vector.h"

#include "pixel_engine/render_manager/components/texture/texture.h"
#include "pixel_engine/render_manager/api/buffer/image.h"

namespace pixel_engine
{
	struct RasterizeTaskDesc
	{
		PEImageBuffer* Target = nullptr;
		const Texture* Texture = nullptr;

		FVector2D StartBase;  // Base position of the first pixel (start + i0*dU + j0*dV)
		FVector2D dU{};         // Step vector along X-axis (horizontal)
		FVector2D dV{};         // Step vector along Y-axis (vertical)

		int i0 = 0;             // Inclusive column start
		int i1 = 0;             // Exclusive column end
		int jA = 0;             // Relative start row of this task
		int jB = 0;             // Relative end row of this task
		int j0Abs = 0;          // Absolute base row offset for the full quad

		int ColsTotal = 0;      // Target width in pixels
		int RowsTotal = 0;      // Target height in pixels

		int TexW = 0;           // Texture width  (cached)
		int TexH = 0;           // Texture height (cached)
	};

	class PFE_API PERasterizeTask
	{
	public:
		PERasterizeTask() noexcept = default;
		explicit PERasterizeTask(const RasterizeTaskDesc& desc) noexcept;

		// Non-copyable, movable
		PERasterizeTask(const PERasterizeTask&)			   = default;
		PERasterizeTask& operator=(const PERasterizeTask&) = default;
		
		PERasterizeTask(PERasterizeTask&&) noexcept;
		PERasterizeTask& operator=(PERasterizeTask&&) noexcept;

		// Execute this task (draw pixels into target).
		void Execute() noexcept;

		// Utility
		bool IsValid() const noexcept;
		std::size_t EstimatedCost() const noexcept; // Optional heuristic (rows * cols)

		// Accessors
		const RasterizeTaskDesc& GetDesc() const noexcept { return m_Desc; }

	private:
		RasterizeTaskDesc m_Desc{};
	};
} // namespace pixel_engine
