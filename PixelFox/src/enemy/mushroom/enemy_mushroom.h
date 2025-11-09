#pragma once

#include "enemy/interface_enemy.h"
#include "ai/controller/chase/chase_ai.h"       
#include "core/vector.h"

#include "pixel_engine/core/event/event_queue.h"
#include "world/state/character_state.h"

#include <memory>
#include <sal.h>
#include <cfloat>

namespace pixel_game
{
    class EnemyMushroom final : public IEnemy
    {
    public:
        EnemyMushroom() = default;
        ~EnemyMushroom() override = default;

        // Lifecycle
        _NODISCARD _Check_return_
            bool Initialize(_In_ const PG_ENEMY_INIT_DESC& desc) override;
        void Update(_In_ float deltaTime) override;
        void Release() override;

        void SetTypeName(const std::string& name) override { m_szEnemyType = name; }
        std::string GetTypeName() const override { return m_szEnemyType; }

        //~ getters
        _NODISCARD _Check_return_
            bool IsActive() const override;

        _NODISCARD _Check_return_ _Ret_maybenull_
            pixel_engine::PEISprite* GetBody() const override;

        _NODISCARD _Check_return_ _Ret_maybenull_
            pixel_engine::AnimSateMachine* GetAnimState() const override;

        _NODISCARD _Check_return_ _Ret_maybenull_
            pixel_engine::BoxCollider* GetCollider() const override;

        _NODISCARD _Check_return_ _Ret_maybenull_
            IAIController* GetController() const override;

        void SetTarget(_In_opt_ pixel_engine::PEISprite* target) override;

        _NODISCARD _Check_return_ _Ret_maybenull_
            pixel_engine::PEISprite* GetTarget() const override;

        _NODISCARD _Check_return_
            bool HasTarget() const override;

        // States
        _NODISCARD _Check_return_
            bool IsDead() const override;

    protected:
        // Initialization helpers
        _NODISCARD _Check_return_
            bool InitEnemyBody(_In_ const PG_ENEMY_INIT_DESC& desc) override;

        _NODISCARD _Check_return_
            bool InitEnemyAnimStateMachine() override;

        _NODISCARD _Check_return_
            bool InitEnemyAI(_In_ const PG_ENEMY_INIT_DESC& desc) override;

        //~ events
        void SubscribeEvents() override;
        void UnSubscribeEvents() override;
        void InitCollisionCallback() override;
        void AddColliderTags() override;

        //~ updates
        void UpdateAnimState(_In_ float deltaTime) override;
        void UpdateAIController(_In_ float deltaTime) override;

    private:
        void SetOnCollisionEnter();
        void SetOnCollisionExit();

    private:
        //~ I HATE HARD CODING THIS!!
        std::string m_szEnemyType{ "NotGiven" };
        std::string m_szBaseFile{ "assets/sprites/Enemy/Mushroom/" };

        //~ Internals
        std::unique_ptr<pixel_engine::QuadObject>      m_pBody{ nullptr };
        std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState{ nullptr };
        std::unique_ptr<ChaseAI>                       m_pController{ nullptr };
        pixel_engine::PEISprite* m_pTarget{ nullptr };

        bool      m_bActive{ false };
        float     m_nHealth{ 50.0f };
        float     m_nMoveSpeedUPS{ 3.5f };
        float     m_nDamage{ 50.f };
        float     m_nKnockBack{ 5.f };
        FVector2D m_scale{ 1.0f, 1.0f };

        //~ subscribed tokens
        fox::vector<pixel_engine::SubToken> m_tokens{};
    };
} // namespace pixel_game
