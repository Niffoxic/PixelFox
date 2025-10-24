#pragma once

#include "PixelFoxEngineAPI.h"
#include "pixel_engine/utilities/id_allocator.h"

#include <string>
#include <sal.h>

namespace pixel_engine
{
	class PFE_API PEIObject
	{
	public:
		PEIObject(): m_idAllocated(IDAllocator<PEIObject>::AllocateID())
		{}

		virtual ~PEIObject() = default;

		//~ ID Accessors
		_NODISCARD _Check_return_
		UniqueId GetInstanceID() const noexcept { return m_idAllocated.ID;   }

		_NODISCARD _Check_return_
		UniqueId GetTypeId    () const noexcept { return m_idAllocated.Type; }

		_NODISCARD _Check_return_
		virtual std::string GetObjectName() const = 0;
		
		_NODISCARD _Check_return_
		virtual bool Initialize() = 0; //~ called once after initalizing
		virtual bool Release   () = 0; // release any upholding resources

	private:
		AllocatedID m_idAllocated;
	};
} // namespace pixel_engine
