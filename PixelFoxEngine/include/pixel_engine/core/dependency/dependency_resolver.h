// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/core/interface/interface_frame.h"

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
		void Register(_In_opt_ IFrameObject* instance);
		void Clear();
		
		_NODISCARD _Check_return_ _Success_(return != false)
		bool Init			();
		bool UpdateLoopStart(_In_ float deltaTime) const;
		bool UpdateLoopEnd  () const;
		bool Shutdown		();

		template<typename... Args>
		void AddDependency(_In_ IFrameObject* late, _In_opt_ Args... early)
		{
			auto& deps = m_connections[late];
			((early ? deps.push_front(early) : void()), ...);
		}

	private:
		_NODISCARD _Check_return_
		fox::list<IFrameObject*> GraphSort();

		void GraphDFS(
			_In_	IFrameObject*							 node,
			_Inout_ fox::unordered_map<IFrameObject*, bool>& visited,
			_Inout_ fox::unordered_map<IFrameObject*, bool>& stack,
			_Inout_ fox::list<IFrameObject*>&				 sorted
		);

	private:
		fox::unordered_map<IFrameObject*, bool>					  m_registeredManagers{};
		fox::unordered_map<IFrameObject*, fox::list<IFrameObject*>>   m_connections		  {};
		fox::list<IFrameObject*>								  m_managerNames	  {};
		fox::list<IFrameObject*>								  m_initOrder		  {};
	};
} // namespace pixel_engine
