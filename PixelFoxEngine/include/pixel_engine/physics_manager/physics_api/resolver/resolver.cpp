#include "pch.h"
#include "resolver.h"

#include <algorithm>
#include <cmath>

#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

void CollisionResolver::ResolveContact(Contact& contact, float deltaTime)
{
	ResolveBoxVsBox(contact, deltaTime);
}

void CollisionResolver::ResolveContact(fox::vector<Contact>& contacts, float deltaTime)
{
	for (auto& c : contacts) ResolveContact(c, deltaTime);
}

void CollisionResolver::ResolveBoxVsBox(Contact& contact, float deltaTime)
{
	if (!contact.A || !contact.B) return;
	
    if (contact.A->IsTrigger() || contact.B->IsTrigger()) return;
    if (contact.A->IsStatic() && contact.B->IsStatic()) return;
    
    ResolvePenetration(contact, deltaTime);
}

void CollisionResolver::ResolvePenetration(Contact& contact, float /*deltaTime*/)
{
    if (contact.Penetration <= 0.f) return;

    const auto typeA = contact.A->GetColliderType();
    const auto typeB = contact.B->GetColliderType();

    auto* rigidbodyA = contact.A->GetRigidBody2D();
    auto* rigidbodyB = contact.B->GetRigidBody2D();

    FVector2D   n     = contact.Normal;
    const float nDot = n.x * n.x + n.y * n.y;

    if (nDot <= 1e-12f) return;
    n *= (1.0f / std::sqrt(nDot)); // no direction found

    const FVector2D ac = contact.A->GetWorldCenter();
    const FVector2D bc = contact.B->GetWorldCenter();

    const FVector2D delta{ bc.x - ac.x, bc.y - ac.y };
    if (delta.x * n.x + delta.y * n.y < 0.f) n = { -n.x, -n.y };

    //~ correction
    constexpr float kSlop    = 0.01f;
    constexpr float kPercent = 0.8f;

    //~ penetration resolve
    const float pen = std::max(contact.Penetration - kSlop, 0.0f);
    if (pen <= 0.f) return;

    const float invA = rigidbodyA->GetInverseMass();
    const float invB = rigidbodyB->GetInverseMass();
    const float invSum = invA + invB;
    const float mag = (pen * kPercent) / invSum;
    const FVector2D corr{ n.x * mag, n.y * mag };

    if (rigidbodyA && !contact.A->IsStatic())
    {
        const auto p = rigidbodyA->GetPosition();
        rigidbodyA->SetPosition({ p.x - corr.x * invA, p.y - corr.y * invA });
    }
    if (rigidbodyB && !contact.B->IsStatic()) 
    {
        const auto p = rigidbodyB->GetPosition();
        rigidbodyB->SetPosition({ p.x + corr.x * invB, p.y + corr.y * invB });
    }
}
