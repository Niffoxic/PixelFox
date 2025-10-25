#pragma once
#include "PixelFoxEngineAPI.h"

#include <cstddef>
#include <windows.h>

#include "core/vector.h"
#include "pixel_engine/core/types.h"

namespace pixel_engine
{
	class PFE_API PEImageBuffer
	{
	public:
		using image_data = unsigned char*;
		using const_image_data = const unsigned char*;

	public:

		_Success_(return != false)
		explicit PEImageBuffer(_In_ const PE_IMAGE_BUFFER_DESC& desc);

		//~ Write to only single channel
		_Success_(return != false)
		bool WriteAt_R(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8_UINT& r);

		_Success_(return != false)
		bool WriteAt_G(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8_UINT& g);

		_Success_(return != false)
		bool WriteAt_B(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8_UINT& b);

		//~ Write to two channels
		_Success_(return != false)
		bool WriteAt_RG(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8G8_UINT& rg);

		_Success_(return != false)
		bool WriteAt_RB(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8G8_UINT& rb);

		_Success_(return != false)
		bool WriteAt_GB(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8G8_UINT& gb);

		//~ Write full RGB color
		_Success_(return != false)
		bool WriteAt(
			_In_ size_t atRow,
			_In_ size_t atColumn,
			_In_ const PFE_FORMAT_R8G8B8_UINT& rgb);

		//~ Clear the image buffer to a color
		_Success_(return != false)
		void ClearImageBuffer(_In_ const PFE_FORMAT_R8G8B8_UINT& color);

		//~ Check if pixel coordinate is inside the buffer
		_NODISCARD _Success_(return != false)
		bool IsInside(_In_ size_t atRow, _In_ size_t atColumn) const noexcept;

		//~ Get pointer to pixel at (row, col)
		_Ret_maybenull_ _Post_writable_byte_size_(3)
		unsigned char* PixelAt(_In_ size_t atRow, _In_ size_t atColumn) noexcept;

		//~ Data accessors
		_NODISCARD _Ret_notnull_
		__forceinline image_data	  Data()		noexcept { return m_imageData.data(); }

		_NODISCARD _Ret_notnull_
		__forceinline const_image_data Data() const noexcept { return m_imageData.data(); }

		_NODISCARD _Check_return_ 
		__forceinline size_t RowPitch () const noexcept { return m_rowPitch;		  }
		_NODISCARD _Check_return_
		__forceinline UINT   Width    () const noexcept { return m_width;			  }
		_NODISCARD _Check_return_
		__forceinline UINT   Height   () const noexcept { return m_height;			  }
		_NODISCARD _Check_return_
		__forceinline size_t ImageSize() const noexcept { return m_imageSize;		  }
		_NODISCARD _Check_return_
		__forceinline bool Empty	  () const noexcept { return m_imageData.empty(); }

	private:
		UINT                       m_height;
		UINT                       m_width;
		size_t                     m_rowPitch;
		size_t                     m_imageSize;
		fox::vector<unsigned char> m_imageData;
	};
}
