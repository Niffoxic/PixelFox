#include "pch.h"
#include "box_collider.h"

#include <cmath>
#include <algorithm>
#include "contact.h"

using namespace pixel_engine;

BoxCollider::BoxCollider(RigidBody2D* rigidBody)
	: m_pRigidBody(rigidBody)
{
}

void BoxCollider::Update(float deltaTime)
{
    //~ specific callback
    for (const auto& [collider, info] : m_collidersTrack)
    {
        if (info.ColliderEntered && !info.ColliderExited)
        {
            Contact tmp;
            if (!CheckCollision(collider, tmp))
            {
                if (info.fnOnTriggerExit) info.fnOnTriggerExit();
                info.ColliderEntered = false;
                info.ColliderExited  = true;
            }
        }
    }

    //~ general callback
    fox::vector<BoxCollider*> toRemove;
    for (const auto& [collider, flag] : m_hitTrack)
    {
        if (!flag) continue;
        if (!collider) continue;
        Contact tmp;
        if (!CheckCollision(collider, tmp))
        {
            flag = false;
            toRemove.push_back(collider);
            if (m_fnOnHitExitCallback) m_fnOnHitExitCallback(collider);
        }
    }

    for (auto& collider : toRemove) m_hitTrack.erase(collider);
}

bool BoxCollider::CheckCollision(BoxCollider* other, Contact& outContact)
{
    if (!other) return false;

    const FVector2D aCenter = GetWorldCenter();
    const FVector2D bCenter = other->GetWorldCenter();
    const FVector2D aHalf   = GetHalfExtents();
    const FVector2D bHalf   = other->GetHalfExtents();

    if (!(aHalf.x > 0 && aHalf.y > 0 && bHalf.x > 0 && bHalf.y > 0)) {
        return false;
    }

    const FVector2D delta = { bCenter.x - aCenter.x, bCenter.y - aCenter.y };
    const FVector2D adelta = AbsV(delta);
    const FVector2D sumHalf = { aHalf.x + bHalf.x, aHalf.y + bHalf.y };
    const FVector2D overlap = { sumHalf.x - adelta.x, sumHalf.y - adelta.y };

    if (overlap.x <= 0.0f || overlap.y <= 0.0f) 
    {
        m_lastContactNormal = { 0.f, 0.f };
        return false;
    }

    if (overlap.x < overlap.y) 
    {
        outContact.Penetration = overlap.x;
        outContact.Normal = { (delta.x < 0.f) ? -1.f : 1.f, 0.f };
    }
    else 
    {
        outContact.Penetration = overlap.y;
        outContact.Normal = { 0.f, (delta.y < 0.f) ? -1.f : 1.f };
    }

    outContact.Point = 
    {
        aCenter.x + outContact.Normal.x * (outContact.Penetration * 0.5f),
        aCenter.y + outContact.Normal.y * (outContact.Penetration * 0.5f)
    };

    m_lastContactNormal = outContact.Normal;
    return true;
}

void BoxCollider::RegisterCollision(BoxCollider* other, const Contact& contact)
{
	if (!other) return;
	m_lastContactNormal = contact.Normal;
	
    if (m_collidersTrack.contains(other))
    {
        auto& info = m_collidersTrack[other];
        if (!info.ColliderEntered)
        {
            info.ColliderEntered = true;
            info.ColliderExited = false;

            if (info.fnOnTriggerEnter)
            {
                info.fnOnTriggerEnter();
            }
        }
    }

    if (!m_hitTrack.contains(other))
    {
        m_hitTrack[other] = true;

        if (m_fnOnHitEnterCallback) 
        {
            m_fnOnHitEnterCallback(other);
        }
    }
}

void BoxCollider::SetOnHitEnterCallback(std::function<void(BoxCollider*)>&& callback)
{
    m_fnOnHitEnterCallback = std::move(callback);
}

void pixel_engine::BoxCollider::SetOnHitExitCallback(std::function<void(BoxCollider*)>&& callback)
{
    m_fnOnHitExitCallback = std::move(callback);
}

void pixel_engine::BoxCollider::AddCallback(const ON_HIT_CALLBACK& callback)
{
    if (!m_collidersTrack.contains(callback.target))
    {
        CollisionCallBack cb{};
        cb.ColliderEntered  = false;
        cb.ColliderExited   = false;
        cb.fnOnTriggerEnter = callback.m_fnOnTriggerEnter;
        cb.fnOnTriggerExit  = callback.m_fnOnTriggerExit;

        m_collidersTrack[callback.target] = std::move(cb);
    }
}

void BoxCollider::SetScale(const FVector2D& scale) { m_scale = scale; }
FVector2D BoxCollider::GetScale() const { return m_scale; }

void BoxCollider::SetOffset(const FVector2D& offset) { m_offset = offset; }
FVector2D BoxCollider::GetOffset() const { return m_offset; }

void BoxCollider::SetColliderType(ColliderType type) { m_colliderType = type; }
ColliderType BoxCollider::GetColliderType() const { return m_colliderType; }

FTransform2D BoxCollider::GetTransform2D() const { return m_pRigidBody->GetTransform(); }
RigidBody2D* BoxCollider::GetRigidBody2D() const { return m_pRigidBody; }

FVector2D BoxCollider::GetMin() const { return { -m_scale.x, -m_scale.y }; }
FVector2D BoxCollider::GetMax() const { return { m_scale.x,  m_scale.y }; }

FVector2D BoxCollider::GetWorldCenter() const
{
    if (!m_pRigidBody) return { 0, 0 };

    auto pos = m_pRigidBody->GetPosition();
    pos += m_offset;
    
    return pos;
}

FVector2D BoxCollider::GetHalfExtents() const { return m_scale * 0.5; }
FVector2D BoxCollider::GetLastContactNormal() const { return m_lastContactNormal; }

bool BoxCollider::IsTrigger() const { return m_colliderType == ColliderType::Trigger; }
bool BoxCollider::IsDynamic() const { return m_colliderType == ColliderType::Dynamic; }
bool BoxCollider::IsStatic () const { return m_colliderType == ColliderType::Static; }

bool pixel_engine::BoxCollider::HasTag(const std::string& tag)
{
    return m_tags[tag];
}

bool pixel_engine::BoxCollider::AttachTag(const std::string& tag)
{
    m_tags[tag] = true;
    return true;
}

void pixel_engine::BoxCollider::DetachTag(const std::string& tag)
{
    m_tags[tag] = false;
}
