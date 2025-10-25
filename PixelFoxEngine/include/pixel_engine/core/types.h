#pragma once
#include "PixelFoxEngineAPI.h"
#include <type_traits>

namespace pixel_engine
{

	typedef struct _PFE_FORMAT_R8_UINT
	{
		unsigned char Value;
	} PFE_FORMAT_R8_UINT;

	typedef struct _PFE_FORMAT_R8G8_UINT
	{
		PFE_FORMAT_R8_UINT R;
		PFE_FORMAT_R8_UINT G;
	} PFE_FORMAT_R8G8_UINT;

	typedef struct _PFE_FORMAT_R8G8B8_UINT
	{
		PFE_FORMAT_R8_UINT R;
		PFE_FORMAT_R8_UINT G;
		PFE_FORMAT_R8_UINT B;
	} PFE_FORMAT_R8G8B8_UINT;

	typedef struct _PE_IMAGE_BUFFER_DESC
	{
		UINT Height;
		UINT Width;
	} PE_IMAGE_BUFFER_DESC;
} // namespace pixel_engine
