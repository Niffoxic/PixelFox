#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#if defined(PIXELFOXMATH_EXPORTS)
		#define PFM_API __declspec(dllexport)
	#else
		#define PFM_API __declspec(dllimport)
	#endif
#else
	#define PFM_API
#endif
