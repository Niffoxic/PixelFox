#pragma once

#include <string>
#include <sal.h>
#include "pixel_engine/utilities/id_allocator.h"

#include "interface_object.h"

namespace pixel_engine
{
	class PFE_API IFrameObject: public PEIObject
	{
	public:
		//~ Interface rules
		virtual void OnFrameBegin(float deltaTime) = 0;
		virtual void OnFrameEnd()				   = 0;
	};
}
