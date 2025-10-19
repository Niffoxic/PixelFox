#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/core/interface/interface_manager.h"

#include "core/unordered_map.h"
#include "core/list.h"

#include <string>
#include <sal.h>

namespace pixel_engine
{
	/// <summary>
	/// Resolves Dependency for initailization and run process
	/// Initializes in provided relation but run loops from reverse order
	/// </summary>
	class PFE_API DependencyResolver
	{
	public:
		 DependencyResolver() = default;
		~DependencyResolver() = default;

		//~ no copy or move
		DependencyResolver(_In_ const DependencyResolver&) = delete;
		DependencyResolver(_Inout_ DependencyResolver&&)   = delete;

		DependencyResolver& operator=(_In_ const DependencyResolver&)    = delete;
		DependencyResolver& operator=(_Inout_ DependencyResolver&&)      = delete;

		//~ Features
		void Register(_In_opt_ IManager* instance);
		void Clear();
		
		_NODISCARD _Check_return_ _Success_(return != false)
		bool Init			();
		bool UpdateLoopStart(_In_ float deltaTime) const;
		bool UpdateLoopEnd  () const;
		bool Shutdown		();

		template<typename... Args>
		void AddDependency(_In_ IManager* late, _In_opt_ Args... early)
		{
			auto& deps = m_connections[late];
			((early ? deps.push_front(early) : void()), ...);
		}

	private:
		fox::list<IManager*> GraphSort();

		void GraphDFS(
			_In_	IManager*							 node,
			_Inout_ fox::unordered_map<IManager*, bool>& visited,
			_Inout_ fox::unordered_map<IManager*, bool>& stack,
			_Inout_ fox::list<IManager*>&				 sorted
		);

	private:
		fox::unordered_map<IManager*, bool>					  m_registeredManagers{};
		fox::unordered_map<IManager*, fox::list<IManager*>>   m_connections		  {};
		fox::list<IManager*>								  m_managerNames	  {};
		fox::list<IManager*>								  m_initOrder		   {};
	};
} // namespace pixel_engine
