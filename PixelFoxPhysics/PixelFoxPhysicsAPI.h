#pragma once

#if defined(_WIN32) || defined(_WIN64)
	#if defined(PIXELFOXPHYSICS_EXPORTS)
		#define PFP_API __declspec(dllexport)
	#else
		#define PFP_API __declspec(dllimport)
	#endif
#else
	#define PFP_API
#endif
