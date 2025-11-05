#include "application.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "pixel_engine/render_manager/components/texture/allocator/texture_resource.h"
#include "pixel_engine/render_manager/components/texture/allocator/tileset_allocator.h"

#include "pixel_engine/render_manager/components/font/font_allocator.h"

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

    m_object = std::make_unique<pixel_engine::QuadObject>();
    m_object->Initialize();
    m_object->SetLayer(pixel_engine::ELayer::Font);
    FTransform2D T{};
    T.Position = { 0, 0 };
    T.Scale    = { 1, 1 };
    T.Rotation = 0.0f;
    m_object->SetTransform(T);

    pixel_engine::Texture* tex = pixel_engine::FontGenerator
        ::Instance().GetGlyph('J');

    m_object->SetTexture(tex);

    m_font = std::make_unique<pixel_engine::PEFont>();
    m_font->SetPosition({ 200, 100 });
    m_font->SetText("I Love programming");

    pixel_engine::PERenderQueue::Instance().AddFont(m_font.get());
    pixel_engine::PERenderQueue::Instance().AddSprite(m_object.get());
}

void pixel_game::Application::Tick(float deltaTime)
{
    m_time += deltaTime;

    auto cam = pixel_engine::PERenderQueue::Instance().GetCamera();
    
    pixel_engine::PFE_WORLD_SPACE_DESC desc{};
    desc.pCamera = cam;
    desc.Origin  = cam->WorldToCamera({ 0.0f, 0.0f }, 32);
    desc.X1      = cam->WorldToCamera({ 1.0f, 0.0f }, 32);
    desc.Y1      = cam->WorldToCamera({ 0.0f, 1.0f }, 32);

    m_object->Update(deltaTime, desc);
}

void pixel_game::Application::Release()
{
}
