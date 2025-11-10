#include "buff_spawner.h"
#include "pixel_engine/utilities/logger/logger.h"
#include <algorithm>

#include "world/events/event_buff.h"

using namespace pixel_game;

void pixel_game::BuffSpawner::Initialize()
{
    if (m_initialized) return;

    const char* paths[]
    {
        "assets/buff/attack_speed_boost/",
        "assets/buff/cd_boost/",
        "assets/buff/damage_boost/",
        "assets/buff/hp_boost/",
        "assets/buff/player_speed_boost/",
    };
    constexpr int kNumPaths = static_cast<int>(std::size(paths));

    m_pool.reserve(kPoolSize);
    m_active.assign(kPoolSize, false);

    for (std::size_t i = 0; i < kPoolSize; ++i)
    {
        const std::string basePath = paths[i % kNumPaths];

        INIT_OBSTICLE_DESC desc{};
        desc.szName = std::format("Buff_{}", i);
        desc.baseTexture = basePath + "0.png";
        desc.obsticleSprites = basePath;           
        desc.scale = { 1.0f, 1.0f };
        desc.position = { 0.0f, 0.0f };

        auto obs = std::make_unique<Obsticle>();
        if (!obs->Init(desc))
        {
            pixel_engine::logger::error(
                "BuffSpawner::Initialize: Failed to init buff '{}'",
                desc.szName);
            continue;
        }

        if (auto* col = obs->GetCollider())
        {
            col->AttachTag("Buff");
            col->AttachTag("buff");
            col->SetColliderType(pixel_engine::ColliderType::Trigger);
        }

        obs->Hide();
        m_pool.push_back(std::move(obs));
    }

    Subscribe();
    m_initialized = true;

    pixel_engine::logger::debug("BuffSpawner initialized {} buff objects.",
        m_pool.size());
}

void BuffSpawner::Update(float /*deltaTime*/)
{
    // no-op for now
}

void BuffSpawner::DeactivateAll()
{
    for (std::size_t i = 0; i < m_pool.size(); ++i)
        Deactivate(static_cast<int>(i));
}

void BuffSpawner::Subscribe()
{
    m_token = pixel_engine::EventQueue::Subscribe<EVENT_DIE_EVENT>(
        [this](const EVENT_DIE_EVENT& ev)
        {
            this->OnEnemyDied(ev);
        }
    );
}

void BuffSpawner::Unsubscribe()
{
    pixel_engine::EventQueue::Unsubscribe(m_token);
}

void BuffSpawner::OnEnemyDied(const EVENT_DIE_EVENT& ev)
{
    const int idx = AcquireFreeIndex();
    if (idx < 0) return;
    ActivateAt(idx, ev.DeathLocation);
}

int BuffSpawner::AcquireFreeIndex() const noexcept
{
    for (std::size_t i = 0; i < m_active.size(); ++i)
        if (!m_active[i]) return static_cast<int>(i);
    return -1;
}

void BuffSpawner::ActivateAt(int idx, const FVector2D& worldPos)
{
    if (idx < 0 || idx >= static_cast<int>(m_pool.size())) return;

    auto& obs = m_pool[static_cast<std::size_t>(idx)];
    if (!obs) return;

    obs->SetPosition(worldPos);
    obs->Draw();
    m_active[static_cast<std::size_t>(idx)] = true;

    WireColliderForIndex(idx);
}

void BuffSpawner::Deactivate(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(m_pool.size())) return;
    auto& obs = m_pool[static_cast<std::size_t>(idx)];
    if (!obs) return;

    obs->Hide();
    m_active[static_cast<std::size_t>(idx)] = false;
}

void BuffSpawner::WireColliderForIndex(int idx)
{
    auto& obs = m_pool[static_cast<std::size_t>(idx)];
    if (!obs) return;

    if (auto* col = obs->GetCollider())
    {
        col->SetOnHitEnterCallback(
            [this, idx](pixel_engine::BoxCollider* other)
            {
                if (!other) return;
                if (!other->HasTag("player")) return;

                PostBuffEventForIndex(other);
                Deactivate(idx);
            });
    }
}

void BuffSpawner::PostBuffEventForIndex(pixel_engine::BoxCollider* /*playerCollider*/)
{
    const int kind = m_nextBuffKind % 5;
    m_nextBuffKind = (m_nextBuffKind + 1) % 5;

    switch (kind)
    {
    case 0:
    {
        PLAYER_HP_BUFF_EVENT ev{ 50.f };
        pixel_engine::EventQueue::Post<PLAYER_HP_BUFF_EVENT>(ev);
    } break;
    case 1:
    {
        PLAYER_SPEED_BUFF_EVENT ev{ 0.25f };
        pixel_engine::EventQueue::Post<PLAYER_SPEED_BUFF_EVENT>(ev);
    } break;
    case 2:
    {
        PLAYER_DAMANGE_BUFF_EVENT ev{ 1.f };
        pixel_engine::EventQueue::Post<PLAYER_DAMANGE_BUFF_EVENT>(ev);
    } break;
    case 3:
    {
        PLAYER_LINEAR_ATTACK_SPEED_BUFF_EVENT ev{ 0.01f };
        pixel_engine::EventQueue::Post<PLAYER_LINEAR_ATTACK_SPEED_BUFF_EVENT>(ev);
    } break;
    case 4:
    default:
    {
        PLAYER_SPECIAL_CD_DEC_EVENT ev{ 0.25f };
        pixel_engine::EventQueue::Post<PLAYER_SPECIAL_CD_DEC_EVENT>(ev);
    } break;
    }
}
