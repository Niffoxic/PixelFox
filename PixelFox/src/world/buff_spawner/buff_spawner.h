#pragma once

#include <memory>
#include <cstddef>
#include <string>
#include "fox_math/vector.h"
#include "core/vector.h"
#include "obsticle/obsiticle.h"
#include "pixel_engine/core/event/event_queue.h"
#include "world/events/enemy_events.h"
#include "world/events/player_events.h"

namespace pixel_game
{
    class BuffSpawner
    {
    public:
        static constexpr std::size_t kPoolSize = 10;

        BuffSpawner() = default;
        ~BuffSpawner() { Unsubscribe(); }

        void Initialize();
        void Update(float deltaTime);

        void DeactivateAll();

    private:
        void Subscribe();
        void Unsubscribe();

        void OnEnemyDied(const EVENT_DIE_EVENT& ev);

        //~ Pool helpers
        int  AcquireFreeIndex() const noexcept;
        void ActivateAt(int idx, const FVector2D& worldPos);
        void Deactivate(int idx);

        void WireColliderForIndex(int idx);
        void PostBuffEventForIndex(pixel_engine::BoxCollider* playerCollider);

    private:
        pixel_engine::SubToken m_token;
        fox::vector<std::unique_ptr<Obsticle>> m_pool{};
        fox::vector<bool>                    m_active{};
        bool                                 m_initialized{ false };

        int                                  m_nextBuffKind{ 0 }; // 0..4
    };
} // namespace pixel_game
