#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#if defined(PIXELFOXENGINE_EXPORTS)
		#define PFE_API __declspec(dllexport)
	#else
		#define PFE_API __declspec(dllimport)
	#endif
#else
	#define PFE_API
#endif
