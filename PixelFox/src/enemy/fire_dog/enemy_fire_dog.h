#pragma once

#include "enemy/interface_enemy.h"
#include "ai/controller/turret/turret_ai.h"
#include "ai/projectile/straight/straight_projectile.h"
#include "core/vector.h"

#include "pixel_engine/core/event/event_queue.h"
#include "world/state/character_state.h"

#include <memory>
#include <sal.h>
#include <cfloat>

namespace pixel_game
{
    class EnemyFireDog final : public IEnemy
    {
    public:
         EnemyFireDog() = default;
        ~EnemyFireDog() override = default;

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

        void SetInvisible() override
        {
            if (m_pBody)
            {
                m_pBody->SetVisible(false);
            }
            if (m_pProjectile)
            {
                m_pProjectile->Deactivate();
                m_pProjectile->SetPosition({ 1000, 1000 });

                if (auto* body = m_pProjectile->GetBody())
                {
                    body->SetVisible(false);
                }
            }
        }


        void  SetHealth(float hp) { m_nHealth = hp; }
        float GetHealth() const { return m_nHealth; }

        void Revive() { m_nHealth = m_nMaxHP; }

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
        std::string m_szBaseFile{ "assets/sprites/Enemy/FireDog/" };

        //~ Internals
        std::unique_ptr<pixel_engine::QuadObject>      m_pBody{ nullptr };
        std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState{ nullptr };
        std::unique_ptr<TurretAI>                      m_pAIController{ nullptr };
        std::unique_ptr<StraightProjectile>            m_pProjectile{ nullptr };
        pixel_engine::PEISprite* m_pTarget{ nullptr };

        bool      m_bActive{ false };
        float     m_nHealth{ 150.0f };
        float m_nMaxHP{ 150.f };
        //~ projectile specifics
        float     m_nAttackDistance{ 50.f };
        float     m_nFireCoolDown{ 0.6f };
        FVector2D m_MuzzleOffset{ 0.5f, 0.f };
        float     m_nProjectileSpeed{ 15.f };
        float     m_nProjectileLifeSpan{ 2.f };

        float     m_nMoveSpeedUPS{ 3.5f };
        float     m_nDamage{ 18.f };
        float     m_nKnockBack{ 5.f };
        FVector2D m_scale{ 2.0f, 2.0f };

        //~ subscribed tokens
        fox::vector<pixel_engine::SubToken> m_tokens{};
    };
} // namespace pixel_game
