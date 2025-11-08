#pragma once

#include "core/unordered_map.h"
#include "core/vector.h"

#include <functional>
#include <memory>
#include <string>

#include "enemy/interface_enemy.h"

namespace pixel_game
{
	class RegistryEnemy
	{
	public:
		using CreateFunctor = std::function<std::unique_ptr<IEnemy>()>;

		static void Register(const std::string& name, CreateFunctor fn)
		{
			// Avoid duplicate registrations
			if (m_registry.contains(name))
				return;

			m_registry[name] = std::move(fn);
			m_ppszNames.push_back(name);
		}

		static std::unique_ptr<IEnemy> CreateEnemy(const std::string& name)
		{
			if (!m_registry.contains(name))
				return nullptr;

			return m_registry[name]();
		}

		static const fox::vector<std::string>& GetEnemyNames()
		{
			return m_ppszNames;
		}

	private:
		inline static fox::unordered_map<std::string, CreateFunctor> m_registry{};
		inline static fox::vector<std::string> m_ppszNames{};
	};

} // namespace pixel_game

#define REGISTER_ENEMY(CLASS_NAME) \
    namespace { \
        struct CLASS_NAME##Registrar { \
            CLASS_NAME##Registrar() { \
                pixel_game::RegistryEnemy::Register(#CLASS_NAME, []() { \
                    auto obj = std::make_unique<CLASS_NAME>(); \
                    obj->SetTypeName(#CLASS_NAME);\
                    return obj; \
                }); \
            } \
        }; \
        static CLASS_NAME##Registrar CLASS_NAME##_registrar; \
    }
