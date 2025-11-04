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
#include "hash_type.h"
#include <atomic>

namespace pixel_engine
{
	using UniqueId = uint64_t;

	struct PFE_API AllocatedID
	{
		UniqueId ID	 { 0u };
		uint64_t Type{ 0u };
	};

	//~ Reserving this [0, 10] for unique cases
	inline _CONSTEXPR20 UniqueId kReservedMax{ 10u };

	_NODISCARD _Check_return_
	inline _CONSTEXPR20 bool IsUniqueIdAssigned(UniqueId id) noexcept
	{
		return id > kReservedMax;
	}

	template<typename Base>
	class IDAllocator
	{
	public:
		_NODISCARD _Check_return_
		static AllocatedID AllocateID() noexcept
		{
			const UniqueId id = m_idAllocator.fetch_add(1, std::memory_order_relaxed) + 1u;
			return { id, TypeHash<Base>() };
		}
	private:
		//~ reserving 0 to 10 for unique cases if I encounter them later
		inline static std::atomic<UniqueId> m_idAllocator{ kReservedMax };
	};
} // namespace pixel_engine
