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
    constexpr int ScreenW = 2000;
    constexpr int ScreenH = 2000;
    constexpr int TilePx = 32;

    const int gridW = (ScreenW + TilePx - 1) / TilePx;
    const int gridH = (ScreenH + TilePx - 1) / TilePx;
    const int COUNT = gridW * gridH;

    const float spacingX = kUnitW;
    const float spacingY = kUnitH;

    const float startX = -0.5f * (gridW * spacingX) + 0.5f * spacingX;
    const float startY = -0.5f * (gridH * spacingY) + 0.5f * spacingY;

    m_objects.clear();
    m_objects.reserve(COUNT);
    m_basePos.resize(COUNT);
    m_amp.resize(COUNT);
    m_speed.resize(COUNT);
    m_phase.resize(COUNT);
    m_moveX.resize(COUNT);

    for (int i = 0; i < COUNT; ++i)
    {
        const int gx = i % gridW;
        const int gy = i / gridW;

        const float px = startX + gx * spacingX;
        const float py = startY + gy * spacingY;

        auto obj = std::make_unique<pixel_engine::QuadObject>();
        obj->Initialize();
        obj->SetUnitSize(kUnitW, kUnitH);
        obj->SetTilePixels(TilePx);
        obj->SetLayer(1);

        FTransform2D T{};
        T.Position = { px, py };
        T.Scale = { 1.0f, 1.0f };
        T.Rotation = 0.0f;
        obj->SetTransform(T);

        pixel_engine::PERenderQueue::Instance().AddSprite(obj.get());
        m_objects.emplace_back(std::move(obj));

        m_basePos[i] = { px, py };
        m_amp[i] = kAmpBase + 0.05f * (i % 13);
        m_speed[i] = kSpeedBase + 0.03f * (i % 17);
        m_phase[i] = 0.5f * static_cast<float>(i);
        m_moveX[i] = ((i & 1) == 0);
    }

    m_objects[0]->SetScale(10, 10);
    m_objects[0]->SetPosition(0, 0);
    pixel_engine::logger::info("Added Objects: {}", m_objects.size());
}

void pixel_game::Application::Tick(float deltaTime)
{
    m_time += deltaTime;
    const float dRot = kRotSpeed * deltaTime;

    auto cam = pixel_engine::PERenderQueue::Instance().GetCamera();
    pixel_engine::PFE_WORLD_SPACE_DESC desc{};
    desc.pCamera = cam;
    desc.Origin = cam->WorldToScreen({ 0.0f, 0.0f }, 32);
    desc.X1 = cam->WorldToScreen({ 1.0f, 0.0f }, 32);
    desc.Y1 = cam->WorldToScreen({ 0.0f, 1.0f }, 32);

    m_objects[0]->Update(deltaTime, desc);
    for (size_t i = 1; i < m_objects.size(); ++i)
    {
        auto& obj = *m_objects[i];

        const float off = m_amp[i] * std::sinf(m_time * m_speed[i] + m_phase[i]);

        FTransform2D T = obj.GetTransform();
        if (m_moveX[i]) {
            T.Position.x = m_basePos[i].x + off;
            T.Position.y = m_basePos[i].y;
        }
        else {
            T.Position.x = m_basePos[i].x;
            T.Position.y = m_basePos[i].y + off;
        }
        T.Rotation += dRot;

        obj.SetTransform(T);
        
        obj.Update(deltaTime, desc);
    }
}

void pixel_game::Application::Release()
{
}
