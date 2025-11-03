#include "application.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

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
    constexpr int TilePx = 32;

    m_object = std::make_unique<pixel_engine::QuadObject>();
    m_object->Initialize();
    m_object->SetUnitSize(kUnitW, kUnitH);
    m_object->SetTilePixels(TilePx);
    m_object->SetLayer(1);
    m_object->SetTexture("assets/sprites/cloud.png");

    FTransform2D T{};
    T.Position = { 0, 0 };
    T.Scale    = { 5.f, 5.f };
    T.Rotation = 0.0f;
    m_object->SetTransform(T);

    pixel_engine::PERenderQueue::Instance().AddSprite(m_object.get());
}

void pixel_game::Application::Tick(float deltaTime)
{
    m_time += deltaTime;
    const float dRot = kRotSpeed * deltaTime;

    auto cam = pixel_engine::PERenderQueue::Instance().GetCamera();
    pixel_engine::PFE_WORLD_SPACE_DESC desc{};
    desc.pCamera = cam;
    desc.Origin  = cam->WorldToScreen({ 0.0f, 0.0f }, 32);
    desc.X1      = cam->WorldToScreen({ 1.0f, 0.0f }, 32);
    desc.Y1      = cam->WorldToScreen({ 0.0f, 1.0f }, 32);

    m_object->Update(deltaTime, desc);
}

void pixel_game::Application::Release()
{
}
