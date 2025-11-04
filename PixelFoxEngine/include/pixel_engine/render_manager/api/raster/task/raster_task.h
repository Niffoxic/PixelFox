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

#include "pixel_engine/render_manager/components/texture/texture.h"
#include "pixel_engine/render_manager/api/buffer/image.h"

namespace pixel_engine
{
	typedef struct _RASTERIZE_TASK_DESC
	{
		_Inout_ PEImageBuffer* target		  = nullptr;
		_Inout_	const Texture* sampledTexture = nullptr;

		_In_ FVector2D startBase {};
		_In_ FVector2D deltaAxisU{};
		_In_ FVector2D deltaAxisV{};

		_In_ int columnStartFrom = 0;
		_In_ int columneEndAt	 = 0;
		_In_ int rowStartFrom	 = 0;
		_In_ int rowEndAt		 = 0;
		_In_ int rowOffset		 = 0;
		_In_ int totalColumns	 = 0;
		_In_ int totalRows		 = 0;
		_In_ int TexWidth		 = 0;
		_In_ int TexHeight		 = 0;

	} RASTERIZE_TASK_DESC;

	class PFE_API PERasterizeTask
	{
	public:
		PERasterizeTask() noexcept = default;
		explicit PERasterizeTask(_In_ const RASTERIZE_TASK_DESC& desc) noexcept;

		// Non-copyable, movable
		PERasterizeTask(_In_ const PERasterizeTask&) = default;
		PERasterizeTask(_Inout_ PERasterizeTask&&) noexcept;

		PERasterizeTask& operator=(_In_ const PERasterizeTask&) = default;
		PERasterizeTask& operator=(_Inout_ PERasterizeTask&&) noexcept;

		// Execute this task (draw pixels into target).
		void Execute() noexcept;

		// Utility
		_NODISCARD _Check_return_
		bool IsValid			 () const noexcept;
		_NODISCARD _Check_return_
		std::size_t EstimatedCost() const noexcept;

		// Accessors
		_NODISCARD _Check_return_
		const RASTERIZE_TASK_DESC& GetDesc() const noexcept { return m_descTask; }

	private:
		RASTERIZE_TASK_DESC m_descTask{};
	};
} // namespace pixel_engine
