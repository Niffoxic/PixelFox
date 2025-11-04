// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "raster_task.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <utility>

using namespace pixel_engine;

_Use_decl_annotations_
PERasterizeTask::PERasterizeTask(const RASTERIZE_TASK_DESC& desc) noexcept
	: m_descTask(desc)
{}

_Use_decl_annotations_
PERasterizeTask::PERasterizeTask(PERasterizeTask&& other) noexcept
	: m_descTask(other.m_descTask)
{
	other.m_descTask.target		     = nullptr;
	other.m_descTask.sampledTexture  = nullptr;
	other.m_descTask.columnStartFrom = 0;
	other.m_descTask.columneEndAt	 = 0;
	other.m_descTask.rowStartFrom	 = 0;
	other.m_descTask.rowEndAt		 = 0;
	other.m_descTask.totalColumns	 = 0;
	other.m_descTask.totalRows		 = 0;
	other.m_descTask.TexWidth		 = 0;
	other.m_descTask.TexHeight		 = 0;
}

_Use_decl_annotations_
PERasterizeTask& PERasterizeTask::operator=(PERasterizeTask&& other) noexcept
{
	if (this != &other)
	{
		m_descTask						 = other.m_descTask;
		other.m_descTask.target			 = nullptr;
		other.m_descTask.sampledTexture  = nullptr;
		other.m_descTask.columnStartFrom = 0;
		other.m_descTask.columneEndAt	 = 0;
		other.m_descTask.rowStartFrom	 = 0;
		other.m_descTask.rowEndAt		 = 0;
		other.m_descTask.totalColumns	 = 0;
		other.m_descTask.totalRows		 = 0;
		other.m_descTask.TexWidth		 = 0;
		other.m_descTask.TexHeight		 = 0;
	}
	return *this;
}

void PERasterizeTask::Execute() noexcept
{
	const auto& d = m_descTask;

	for (int y = d.rowStartFrom; y < d.rowEndAt; ++y)
	{
		const float rowX = d.startBase.x + static_cast<float>(y) * d.deltaAxisV.x;
		const float rowY = d.startBase.y + static_cast<float>(y) * d.deltaAxisV.y;

		const int jAbs = d.rowOffset + y;
		if (static_cast<unsigned>(jAbs) >=
			static_cast<unsigned>(d.totalRows))
			continue;

		for (int x = d.columnStartFrom; x < d.columneEndAt; ++x)
		{
			const float px = rowX + static_cast<float>(x - d.columnStartFrom) * d.deltaAxisU.x;
			const float py = rowY + static_cast<float>(x - d.columnStartFrom) * d.deltaAxisU.y;

			const int ix = static_cast<int>(std::floor(px + 0.5f));
			const int iy = static_cast<int>(std::floor(py + 0.5f));

			const int iAbs = x;
			if (static_cast<unsigned>(iAbs) >= static_cast<unsigned>(d.totalColumns))
				continue;

			if (static_cast<unsigned>(ix) >= static_cast<unsigned>(d.totalColumns) ||
				static_cast<unsigned>(iy) >= static_cast<unsigned>(d.totalRows))
				continue;

			d.target->WriteAt(iy, ix, d.sampledTexture->GetPixel(iAbs, jAbs));
		}
	}
}

_Use_decl_annotations_
bool PERasterizeTask::IsValid() const noexcept
{
	const auto& d = m_descTask;

	const bool validPtr = (d.target != nullptr)		    &&
						  (d.sampledTexture != nullptr);
	
	const bool validCol = (d.totalColumns > 0)				   &&
						  (d.columnStartFrom < d.columneEndAt) &&
						  (d.columnStartFrom >= 0)			   &&
						  (d.columneEndAt <= d.totalColumns);
	
	const bool validRow = (d.totalRows > 0)			    &&
						  (d.rowStartFrom < d.rowEndAt) &&
						  (d.rowStartFrom >= 0);

	const bool validTex = (d.TexWidth > 0)  &&
					      (d.TexHeight > 0);

	return validPtr && validCol && validRow && validTex;
}

_Use_decl_annotations_
std::size_t PERasterizeTask::EstimatedCost() const noexcept
{
	const int cols = std::max(0, m_descTask.columneEndAt - m_descTask.columnStartFrom);
	const int rows = std::max(0, m_descTask.rowEndAt     - m_descTask.rowStartFrom);
	
	return static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows);
}
