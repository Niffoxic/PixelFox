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

		static void RegisterBoss(const std::string& name, CreateFunctor fn)
		{
			if (m_bossEegistry.contains(name)) return;

			m_bossEegistry[name] = std::move(fn);
			m_ppszBossNames.push_back(name);
		}

		static std::unique_ptr<IEnemy> CreateEnemy(const std::string& name)
		{
			if (!m_registry.contains(name))
				return nullptr;

			return m_registry[name]();
		}

		static std::unique_ptr<IEnemy> CreateBoss(const std::string& name)
		{
			if (!m_bossEegistry.contains(name))
				return nullptr;

			return m_bossEegistry[name]();
		}

		static const fox::vector<std::string>& GetEnemyNames()
		{
			return m_ppszNames;
		}

		static const fox::vector<std::string>& GetBossNames()
		{
			return m_ppszBossNames;
		}

	private:
		inline static fox::unordered_map<std::string, CreateFunctor> m_registry{};
		inline static fox::unordered_map<std::string, CreateFunctor> m_bossEegistry{};
		inline static fox::vector<std::string> m_ppszNames{};
		inline static fox::vector<std::string> m_ppszBossNames{};
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

#define REGISTER_BOSS_ENEMY(CLASS_NAME) \
    namespace { \
        struct CLASS_NAME##BossRegistrar { \
            CLASS_NAME##BossRegistrar() { \
                pixel_game::RegistryEnemy::RegisterBoss(#CLASS_NAME, []() { \
                    auto obj = std::make_unique<CLASS_NAME>(); \
                    obj->SetTypeName(#CLASS_NAME); \
                    return obj; \
                }); \
            } \
        }; \
        static CLASS_NAME##BossRegistrar CLASS_NAME##_boss_registrar; \
    }