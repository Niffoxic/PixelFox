#pragma once

#include "PixelFoxEngineAPI.h"

#include <unordered_map>
#include <typeindex>
#include <shared_mutex>
#include <functional>
#include <cassert>
#include <utility>
#include <memory>

#include "core/vector.h"

#include "pixel_engine/core/interface/interface_singleton.h"

namespace pixel_engine
{
	class PFE_API ServiceLocator
	{
		using Deleter = std::function<void()>;
	public:
		ServiceLocator() = default;

		//~ Create Services
		template<class CType, class...Args>
		_Check_return_opt_
		static CType* CreateService(Args&&...args)
		{
			const std::type_index type = typeid(CType);
			std::unique_lock lock(s_mutex);
			if (s_services.contains(type)) return static_cast<CType*>(s_services[type]);

			// create and own
			CType* instance = new CType(std::forward<Args>(args)...);
			s_services[type] = static_cast<void*>(instance);
			s_deleters.emplace_back([type, instance]()
			{
				delete instance;
				s_services.erase(type);
			});
			return instance;
		}

		//~ Registers Instance by class type id and pointer
		template<class T>
		static void RegisterInstance(
			_In_ T* instance,
			_In_opt_ Deleter deleter = {}
		)
		{
			assert(instance && "RegisterInstacne called with nullptr!");
			const std::type_index key{ typeid(T) }; //~ to save by class name

			if (s_services.contains(key)) return;

			std::unique_lock lock(s_mutex);

			s_services[key] = static_cast<void*>(instance);
			s_deleters.emplace_back([key, d = std::move(deleter)]()
			{
				if (d) d();
				s_services.erase(key);
			});
		}

		template<class CSingleton, bool LeakOnExit = false, class...Args>
		static void RegisterSingleton(_In_opt_ Args&&... args)
		{
			using Base = ISingleton<CSingleton, LeakOnExit>;

			if _CONSTEXPR20(sizeof...(Args) > 0) //~ if args are there
			{
				(void)Base::Init(std::forward<Args>(args)...);
			}
			else
			{
				(void)Base::Instance(); //~ we can init a default constructor
			}

			CSingleton* ptr = Base::TryGet();

			std::string message = std::string("For some reason ") + typeid(CSingleton).name() + " isnt initialized!";
			assert(ptr && message.c_str());

			const std::type_index key{ typeid(CSingleton) };

			std::unique_lock lock(s_mutex);
			if (s_services.contains(key)) return;

			s_services[key] = static_cast<void*>(ptr);
			s_deleters.emplace_back([]()
			{
				if _CONSTEXPR20(!LeakOnExit) Base::Destroy();
			});
		}

		template<class T>
		_Ret_maybenull_ _Success_(return != nullptr)
		static T* Get()
		{
			const std::type_index key{ typeid(T) };
			std::shared_lock lock(s_mutex);

			if (!s_services.contains(key)) return nullptr;

			return static_cast<T*>(s_services[key]);
		}

		template<class T>
		static void Reset()
		{
			const std::type_index key{ typeid(T) };

			std::unique_lock lock(s_mutex);

			if (s_services.contains(key))
			{
				s_services.erase(key);
			}
		}

		static void Shutdown() noexcept
		{
			std::unique_lock lock(s_mutex);

			for (auto it = s_deleters.rbegin(); it != s_deleters.rend(); ++it)
			{
				try { (*it)(); }
				catch (...)
				{ // TODO: Log Failture to call deleter
				}
			}
			s_deleters.clear();
			s_services.clear();
		}

		template<class T>
		_NODISCARD _Check_return_
		static bool HasService() noexcept
		{
			std::type_index type{ typeid(T) };

			std::shared_lock lock(s_mutex);
			return s_services.contains(type);
		}

	private:
		static std::shared_mutex						  s_mutex;
		static fox::vector<Deleter>						  s_deleters;
		static std::unordered_map<std::type_index, void*> s_services;
	};

	//~ Helpers
	//~ just registers already initialized or created singleton object
	template<class CSingleton>
	void RegisterExistingSingleton(CSingleton& singleton)
	{
		ServiceLocator::RegisterInstance<CSingleton>(&singleton, {});
	}

	//~ Creates and registers a service
	template<class CType, class... Args>
	CType* MakeService(Args&&... args)
	{
		return ServiceLocator::CreateService<CType>(std::forward<Args>(args)...);
	}
}

// Creates and registers a service owned by ServiceLocator.
// use it like: auto* rd = SERVICE_CREATE(RenderDevice, hwnd, desc)
#define FOX_SERVICE_CREATE(classname, ...) \
    pixel_engine::MakeService<classname>(__VA_ARGS__)

// Registers an existing instance
// use it like: SERVICE_REIGSTER(FutureServices, &SomeFutureServices)
#define FOX_SERVICE_REIGSTER(classname, instance_ptr) \
    pixel_engine::ServiceLocator::RegisterInstance<classname>((instance_ptr), {})

// Returns a pointer to a registered service, or nullptr if not found
// use it like: auto* rd = SERVICE_FIND(RenderDevice);
#define FOX_SERVICE_FIND(classname) \
    (pixel_engine::ServiceLocator::Get<classname>())

// Registers a singleton class
// use it like: SERVICE_REGISTER_SINGLETON(RenderDevice, hwnd, init_desc)
#define FOX_SERVICE_REGISTER_SINGLETON(classname, ...) \
    pixel_engine::ServiceLocator::RegisterSingleton<classname>(__VA_ARGS__)
