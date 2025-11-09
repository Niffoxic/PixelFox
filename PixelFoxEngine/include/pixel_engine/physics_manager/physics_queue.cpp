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

bool pixel_engine::PhysicsQueue::FrameBegin(float delatTime)
{
    pixel_engine::PFE_WORLD_SPACE_DESC desc{};
    desc.pCamera = m_pCamera;
    desc.Origin  = m_pCamera->WorldToCamera({ 0, 0 }, 32);
    desc.X1      = m_pCamera->WorldToCamera({ 1.0f, 0.0f }, 32);
    desc.Y1      = m_pCamera->WorldToCamera({ 0.0f, 1.0f }, 32);

    fox::vector<BoxCollider*> colliders;
    for (const auto& obj : m_sprites)
    {        
        auto* sprite = obj.second;

        if (sprite && sprite->NeedSampling())
        {
            pixel_engine::PFE_CREATE_SAMPLE_TEXTURE desc{};
            desc.texture = sprite->GetTexture();
            desc.scaledBy = sprite->GetScale();
            desc.tileSize = 32;
            if (!desc.texture) continue;
            auto* built = Sampler::Instance().BuildTexture(desc);
            sprite->AssignSampledTexture(built);
        }
        if (!obj.second->IsVisible()) continue;

        obj.second->Update(delatTime, desc);

        if (auto* rigidBody = obj.second->GetRigidBody2D())
        {
            rigidBody->Integrate(delatTime);
        }

        if (auto* collider = obj.second->GetCollider())
        {
            collider->Update(delatTime);
            colliders.push_back(collider);
        }
    }
    
    fox::vector<Contact> contacts{};
    for (int i = 0; i < colliders.size(); i++)
    {
        for (int j = i + 1; j < colliders.size(); j++) 
        {
            BoxCollider* a = colliders[i];
            BoxCollider* b = colliders[j];

            if (!a || !b) continue;
            Contact contact{};
            contact.A = a;
            contact.B = b;

            if (contact.A->HasTag("Enemy") && contact.B->IsStatic()) continue;
            if (contact.B->HasTag("Enemy") && contact.A->IsStatic()) continue;

            if (a->CheckCollision(b, contact))
            {
                a->RegisterCollision(b, contact);
                b->RegisterCollision(a, contact);
                contacts.push_back(contact);
            }
        }
    }

    CollisionResolver::ResolveContact(contacts, delatTime);

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
