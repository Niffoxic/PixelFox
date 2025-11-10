// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "physics_queue.h"

#include "physics_api/resolver/resolver.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/render_manager/render_queue/sampler/sample_allocator.h"

bool pixel_engine::PhysicsQueue::Initialize(Camera2D* camera)
{
    m_pCamera = camera;
    return true;
}

bool pixel_engine::PhysicsQueue::FrameBegin(float deltaTime)
{
    try
    {
        pixel_engine::PFE_WORLD_SPACE_DESC desc{};
        desc.pCamera = m_pCamera;
        desc.Origin = m_pCamera->WorldToCamera({ 0, 0 }, 32);
        desc.X1 = m_pCamera->WorldToCamera({ 1.0f, 0.0f }, 32);
        desc.Y1 = m_pCamera->WorldToCamera({ 0.0f, 1.0f }, 32);

        fox::vector<BoxCollider*> colliders;
        colliders.reserve(m_sprites.size());

        for (const auto& obj : m_sprites)
        {
            auto* sprite = obj.second;
            if (!sprite) continue;

            if (sprite->NeedSampling())
            {
                pixel_engine::PFE_CREATE_SAMPLE_TEXTURE tdesc{};
                tdesc.texture = sprite->GetTexture();
                tdesc.scaledBy = sprite->GetScale();
                tdesc.tileSize = 32;

                if (tdesc.texture)
                {
                    auto* built = Sampler::Instance().BuildTexture(tdesc);
                    sprite->AssignSampledTexture(built);
                }
            }

            if (!sprite->IsVisible()) continue;

            sprite->Update(deltaTime, desc);

            if (auto* rigidBody = sprite->GetRigidBody2D())
            {
                rigidBody->Integrate(deltaTime);
            }

            if (auto* collider = sprite->GetCollider())
            {
                collider->Update(deltaTime);
                colliders.push_back(collider);
            }
        }

        fox::vector<Contact> contacts{};
        contacts.reserve(colliders.size());

        for (int i = 0; i < static_cast<int>(colliders.size()); i++)
        {
            for (int j = i + 1; j < static_cast<int>(colliders.size()); j++)
            {
                BoxCollider* a = colliders[i];
                BoxCollider* b = colliders[j];
                if (!a || !b) continue;

                Contact contact{};
                contact.A = a;
                contact.B = b;

                // skip unwanted cases
                if (a->HasTag("Enemy") && b->IsStatic()) continue;
                if (b->HasTag("Enemy") && a->IsStatic()) continue;

                try
                {
                    if (a->CheckCollision(b, contact))
                    {
                        a->RegisterCollision(b, contact);
                        b->RegisterCollision(a, contact);
                        contacts.push_back(contact);
                    }
                }
                catch (const std::exception& e)
                {
                    pixel_engine::logger::error(
                        "PhysicsQueue::FrameBegin - Exception in CheckCollision(A={}, B={}): {}",
                        static_cast<const void*>(a),
                        static_cast<const void*>(b),
                        e.what());
                }
                catch (...)
                {
                    pixel_engine::logger::error(
                        "PhysicsQueue::FrameBegin - Unknown exception in CheckCollision(A={}, B={})",
                        static_cast<const void*>(a),
                        static_cast<const void*>(b));
                }
            }
        }

        try
        {
            CollisionResolver::ResolveContact(contacts, deltaTime);
        }
        catch (const std::exception& e)
        {
            pixel_engine::logger::error(
                "PhysicsQueue::FrameBegin - Exception in ResolveContact: {}", e.what());
        }
        catch (...)
        {
            pixel_engine::logger::error(
                "PhysicsQueue::FrameBegin - Unknown exception in ResolveContact()");
        }
    }
    catch (const std::exception& e)
    {
        pixel_engine::logger::error("PhysicsQueue::FrameBegin - Exception: {}", e.what());
    }
    catch (...)
    {
        pixel_engine::logger::error("PhysicsQueue::FrameBegin - Unknown top-level exception");
    }

    return true;
}

bool pixel_engine::PhysicsQueue::FrameEnd()
{
    return true;
}

void pixel_engine::PhysicsQueue::OnRelease()
{
    Clear();
}

void pixel_engine::PhysicsQueue::UpdateSprite(float deltaTime, const pixel_engine::PFE_WORLD_SPACE_DESC& desc)
{
    for (const auto& obj : m_sprites)
    {
        obj.second->Update(deltaTime, desc);
    }
}

bool pixel_engine::PhysicsQueue::AddObject(PEISprite* sprite)
{
    UniqueId id = sprite->GetInstanceID();
    if (m_sprites.contains(id)) return false;

    m_sprites[id] = sprite;

    PERenderQueue::Instance().AddSprite(sprite);

    return true;
}

bool pixel_engine::PhysicsQueue::RemoveObject(PEISprite* sprite)
{
    if (not sprite) return false;
    return RemoveObject(sprite->GetInstanceID());
}

bool pixel_engine::PhysicsQueue::RemoveObject(UniqueId id)
{
    if (!m_sprites.contains(id)) return false;
    m_sprites.erase(id);
    PERenderQueue::Instance().RemoveSprite(id);
    return true;
}

void pixel_engine::PhysicsQueue::Clear()
{
    m_sprites.clear();
}
