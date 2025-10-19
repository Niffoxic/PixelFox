#pragma once

#include <string>
#include <sal.h>
#include "pixel_engine/utilities/id_allocator.h"

namespace pixel_engine
{
	class __declspec(novtable) PFE_API IManager
	{
	public:
		IManager()
			: m_idAllocated(IDAllocator<IManager>::AllocateID())
		{}

		virtual ~IManager() = default;

		IManager(const IManager&) = delete;
		IManager(IManager&&)	  = delete;

		IManager& operator=(const IManager&) = delete;
		IManager& operator=(IManager&&)		 = delete;

		//~ Interface rules
		_NODISCARD _Check_return_ virtual bool		  OnInit()				 = 0;
		_NODISCARD _Check_return_ virtual bool		  OnRelease()			 = 0;
		_NODISCARD _Check_return_ virtual std::string GetManagerName() const = 0;

		virtual void OnLoopStart(float deltaTime) = 0;
		virtual void OnLoopEnd()				  = 0;

		//~ ID Accessors
		_NODISCARD _Check_return_
		UniqueId GetInstanceId() const noexcept { return m_idAllocated.ID;   }
		
		_NODISCARD _Check_return_
		UniqueId GetTypeId	  () const noexcept { return m_idAllocated.Type; }

	protected:
		AllocatedID m_idAllocated;
	};
}
