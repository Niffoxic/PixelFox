#include "application.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"

namespace
{
    constexpr int   kTilePx         = 32;
    constexpr float kUnitW          = 1.0f;
    constexpr float kUnitH          = 1.0f;
    constexpr float kAmp            = 1.5f;
    constexpr float kSpeed          = 4.7f;
    constexpr float kPhase          = 0.0f;
    constexpr float kRotSpeed       = 0.75f;
} // test

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
    // base transform
    m_obj.SetPosition(0.0f, 0.0f);
    m_obj.SetScale(kUnitW, kUnitH);
    m_obj.SetRotation(0.0f);
    m_obj.SetLayer(0);
    m_obj.SetUnitSize(kUnitW, kUnitH);
    m_obj.SetTilePixels(kTilePx);

    m_basePos = { 0.0f, 0.0f };
    m_moveX = true;
    m_obj.MarkDirty(true);

    pixel_engine::PERenderQueue::Instance().AddSprite(&m_obj);
}

void pixel_game::Application::Tick(float deltaTime)
{
    static float m_time = 0.0f;
    m_time += deltaTime;

    const float off = kAmp * std::sin(m_time * kSpeed + kPhase);

    auto T = m_obj.GetTransform();
    if (m_moveX)
    {
        T.Position.x = m_basePos.x + off;
        T.Position.y = m_basePos.y;
    }
    else 
    {
        T.Position.x = m_basePos.x;
        T.Position.y = m_basePos.y + off;
    }
    T.Rotation += kRotSpeed * deltaTime;

    m_obj.SetTransform(T);
}

void pixel_game::Application::Release()
{
}
