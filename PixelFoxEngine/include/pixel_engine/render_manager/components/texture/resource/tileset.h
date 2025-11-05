#pragma once

#include "PixelFoxEngineAPI.h"

#include "texture.h"
#include <memory>

namespace pixel_engine
{
    typedef struct _CREATE_TILE_SET_DESC
    {
        _In_ FVector2D scale;
        _In_ unsigned  sliceHeight;
        _In_ unsigned  sliceWidth;
        _In_ unsigned  margin  = 0;
        _In_ unsigned  spacing = 0;
    } CREATE_TILE_SET_DESC;

	class PFE_API TileSet
	{
	public:
		TileSet() = default;
		explicit TileSet(Texture* source);

        bool Slice(_In_ const CREATE_TILE_SET_DESC& desc);

        TileSet(const TileSet&)                 = delete;
        TileSet& operator=(const TileSet&)      = delete;
        TileSet(TileSet&&) noexcept             = default;
        TileSet& operator=(TileSet&&) noexcept  = default;

        _NODISCARD size_t   GetSliceCount() const noexcept { return m_ppSampledSlices.size(); }
        _NODISCARD Texture* GetSlice(_In_ size_t index) const noexcept;
        _NODISCARD Texture* GetSlice(_In_ unsigned row, _In_ unsigned col) const noexcept;

        _NODISCARD unsigned GetRows() const noexcept { return m_Rows; }
        _NODISCARD unsigned GetCols() const noexcept { return m_Cols; }

        void LogSlicesInfo() const;

    private:
        Texture* m_pSourceTexture{ nullptr };
        std::vector<std::unique_ptr<Texture>> m_ppSlices;
        std::vector<Texture*> m_ppSampledSlices;

        unsigned m_Rows{ 0 };
        unsigned m_Cols{ 0 };
        unsigned m_SliceW{ 0 };
        unsigned m_SliceH{ 0 };

	};
} // namespace pixel_engine
