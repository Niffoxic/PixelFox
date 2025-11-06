#include "application.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "pixel_engine/render_manager/components/texture/allocator/texture_resource.h"
#include "pixel_engine/render_manager/components/texture/allocator/tileset_allocator.h"

#include "pixel_engine/render_manager/components/font/font_allocator.h"
#include "pixel_engine/physics_manager/physics_queue.h"

_Use_decl_annotations_
pixel_game::Application::Application(
	pixel_engine::PIXEL_ENGINE_CONSTRUCT_DESC const* desc)
	: pixel_engine::PixelEngine(desc)
{}

pixel_game::Application::~Application()
{}

_Use_decl_annotations_
bool pixel_game::Application::InitApplication(pixel_engine::PIXEL_ENGINE_INIT_DESC const* desc)
{
	return true;
}

void pixel_game::Application::BeginPlay()
{
    //~ font
    m_font = std::make_unique<pixel_engine::PEFont>();
    m_font->SetPosition({ 200, 100 });
    m_font->SetText("I Love programming");

    pixel_engine::PERenderQueue::Instance().AddFont(m_font.get());

    //~ object 1
    m_object = std::make_unique<pixel_engine::QuadObject>();
    m_object->Initialize();
    m_object->SetLayer(pixel_engine::ELayer::Font);
    FTransform2D T{};
    T.Position = { -5, 0 };
    T.Scale    = { 1, 1 };
    T.Rotation = 0.0f;
    m_object->SetTransform(T);
    m_object->GetCollider()
        ->SetColliderType(pixel_engine::ColliderType::Dynamic);

    pixel_engine::Texture* tex = pixel_engine::FontGenerator
        ::Instance().GetGlyph('J');

    m_object->SetTexture(tex);

    pixel_engine::PhysicsQueue::Instance().AddObject(m_object.get());

    //~ Object 2
    m_object1 = std::make_unique<pixel_engine::QuadObject>();
    m_object1->Initialize();
    m_object1->SetLayer(pixel_engine::ELayer::Font);
    T.Position = { 5, 0 };
    T.Scale    = { 1, 1 };
    T.Rotation = 0.0f;
    m_object1->SetTransform(T);

    tex = pixel_engine::FontGenerator::Instance().GetGlyph('O');

    m_object1->SetTexture(tex);
    m_object1->GetCollider()
        ->SetColliderType(pixel_engine::ColliderType::Static);

    pixel_engine::PhysicsQueue::Instance().AddObject(m_object1.get());

    //~ callback test
    pixel_engine::ON_HIT_CALLBACK callback{};
    callback.m_fnOnTriggerEnter = [&]()
    {
        m_object1->GetRigidBody2D()->SetVelocity({ 0.f, 0.f });
        pixel_engine::logger::debug("I Got hit!!");
    };
    callback.m_fnOnTriggerExit = []()
    {
        pixel_engine::logger::debug("left the body");
    };
    callback.target = m_object->GetCollider();

    m_object1->GetCollider()->AddCallback(callback);
    m_object1->GetRigidBody2D()->SetLinearDamping(0.6f);
    m_object1->GetCollider()->AttachTag("Enemy");
    m_object->GetRigidBody2D()->SetLinearDamping(0.6f);

    m_object->GetCollider()->SetOnHitEnterCallback(
    [](pixel_engine::BoxCollider* collider)
    {
        if (collider->HasTag("EnemyA"))
        pixel_engine::logger::debug("Enemy Hurting me");
        else pixel_engine::logger::debug("Unknown tag");
    });

    m_object->GetCollider()->SetOnHitExitCallback(
    [](pixel_engine::BoxCollider* collider)
    {
        if (collider->HasTag("EnemyA"))
        pixel_engine::logger::debug("Enemy stopped Hurting me");
        else pixel_engine::logger::debug("Unknown tag");
    });
}

void pixel_game::Application::Tick(float deltaTime)
{    
    m_time += deltaTime;
    static bool once = false;
    static bool bounce = false;
    if (m_time >= 6.f && not once)
    {
        m_object->GetRigidBody2D()->AddVelocity({ 13.f, 0 });
        once = true;
    }
    if (once && m_time >= 10.f && not bounce)
    {
        bounce = true;
        m_object->GetRigidBody2D()->AddVelocity({ -15.f, 0 });
    }
}

void pixel_game::Application::Release()
{
}
