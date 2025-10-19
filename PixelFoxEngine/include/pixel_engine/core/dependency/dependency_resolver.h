#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_manager.h"

#include "core/unordered_map.h"
#include "core/vector.h"

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
		DependencyResolver& operator=(_Inout_ const DependencyResolver&) = delete;

		//~ Features
		void Register(_In_opt_ IManager* instance);
		void Clear();
		
		_NODISCARD _Check_return_ _Success_(return != false)
		bool Init			();
		bool UpdateLoopStart(_In_ float deltaTime) const;
		bool UpdateLoopEnd  () const;
		bool Shutdown		();

		template<typename...Args>
		void AddDependency(_In_ IManager* late, _In_opt_ Args&&...early)
		{
			(m_connections.insert_or_assign(late, early)...);
		}

	private:
		fox::vector<IManager*> GraphSort();

		void GraphDFS(
			_In_ const IManager*						 node,
			_Inout_ fox::unordered_map<IManager*, bool>& visited,
			_Inout_ fox::unordered_map<IManager*, bool>& stack,
			_In_ fox::vector<IManager*>					 sorted
		);

	private:
		fox::unordered_map<IManager*, bool>					 m_registeredManagers{};
		fox::unordered_map<IManager*, fox::vector<IManager>> m_connections		 {};
		fox::vector<IManager*>								 m_managerNames		 {};
		fox::vector<IManager*>								 m_initOrder		 {};
	};
} // namespace pixel_engine
