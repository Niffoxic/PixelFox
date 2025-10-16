#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#if defined(PIXELFOXCORE_EXPORTS)
		#define PFC_API __declspec(dllexport)
	#else
		#define PFC_API __declspec(dllimport)
	#endif
#else
	#define PFC_API
#endif
