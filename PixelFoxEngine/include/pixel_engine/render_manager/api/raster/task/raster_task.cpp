#include "pch.h"
#include "raster_task.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <utility>

namespace pixel_engine
{

	PERasterizeTask::PERasterizeTask(const RasterizeTaskDesc& desc) noexcept
		: m_Desc(desc)
	{
	}

	PERasterizeTask::PERasterizeTask(PERasterizeTask&& other) noexcept
		: m_Desc(other.m_Desc)
	{
		other.m_Desc.Target = nullptr;
		other.m_Desc.Texture = nullptr;
		other.m_Desc.i0 = other.m_Desc.i1 = 0;
		other.m_Desc.jA = other.m_Desc.jB = 0;
		other.m_Desc.ColsTotal = other.m_Desc.RowsTotal = 0;
		other.m_Desc.TexW = other.m_Desc.TexH = 0;
	}

	PERasterizeTask& PERasterizeTask::operator=(PERasterizeTask&& other) noexcept
	{
		if (this != &other)
		{
			m_Desc = other.m_Desc;

			other.m_Desc.Target = nullptr;
			other.m_Desc.Texture = nullptr;
			other.m_Desc.i0 = other.m_Desc.i1 = 0;
			other.m_Desc.jA = other.m_Desc.jB = 0;
			other.m_Desc.ColsTotal = other.m_Desc.RowsTotal = 0;
			other.m_Desc.TexW = other.m_Desc.TexH = 0;
		}
		return *this;
	}

	void PERasterizeTask::Execute() noexcept
	{
		const auto& d = m_Desc;

		for (int jRel = d.jA; jRel < d.jB; ++jRel)
		{
			const float rowX = d.StartBase.x + static_cast<float>(jRel) * d.dV.x;
			const float rowY = d.StartBase.y + static_cast<float>(jRel) * d.dV.y;

			const int jAbs = d.j0Abs + jRel;
			if (static_cast<unsigned>(jAbs) >= static_cast<unsigned>(d.RowsTotal))
				continue;

			for (int i = d.i0; i < d.i1; ++i)
			{
				const float px = rowX + static_cast<float>(i - d.i0) * d.dU.x;
				const float py = rowY + static_cast<float>(i - d.i0) * d.dU.y;

				const int ix = static_cast<int>(std::floor(px + 0.5f));
				const int iy = static_cast<int>(std::floor(py + 0.5f));

				const int iAbs = i;
				if (static_cast<unsigned>(iAbs) >= static_cast<unsigned>(d.ColsTotal))
					continue;

				if (static_cast<unsigned>(ix) >= static_cast<unsigned>(d.ColsTotal) ||
					static_cast<unsigned>(iy) >= static_cast<unsigned>(d.RowsTotal))
					continue;

				d.Target->WriteAt(iy, ix, d.Texture->GetPixel(iAbs, jAbs));
			}
		}
	}

	bool PERasterizeTask::IsValid() const noexcept
	{
		const auto& d = m_Desc;

		const bool ptrsOk = (d.Target != nullptr) && (d.Texture != nullptr);
		const bool colsOk = (d.ColsTotal > 0) && (d.i0 < d.i1) &&
			(d.i0 >= 0) && (d.i1 <= d.ColsTotal);
		const bool rowsOk = (d.RowsTotal > 0) && (d.jA < d.jB) &&
			(d.jA >= 0);
		const bool texOk = (d.TexW > 0) && (d.TexH > 0);

		return ptrsOk && colsOk && rowsOk && texOk;
	}

	std::size_t PERasterizeTask::EstimatedCost() const noexcept
	{
		const int cols = std::max(0, m_Desc.i1 - m_Desc.i0);
		const int rows = std::max(0, m_Desc.jB - m_Desc.jA);
		return static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows);
	}

} // namespace pixel_engine
