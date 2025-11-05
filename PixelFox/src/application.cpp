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
    m_object->SetLayer(pixel_engine::ELayer::Background);
    FTransform2D T{};
    T.Position = { 0, 0 };
    T.Scale    = { 50, 50 };
    T.Rotation = 0.0f;
    m_object->SetTransform(T);

    m_object->SetTexture("assets/sprites/test.png");
    
    pixel_engine::PERenderQueue::Instance().AddSprite(m_object.get());

    for (int i = 0; i < 400; i++)
    {
        auto obj = std::make_unique<pixel_engine::QuadObject>();
        obj->Initialize();
        obj->SetLayer(pixel_engine::ELayer::Obstacles);

        int y = (i / 40) - 20;
        int x = (i % 40) - 20;
        FTransform2D res;
        res.Position.x = static_cast<float>(x);
        res.Position.y = static_cast<float>(y);
        res.Scale = { 1.f, 1.f };
        res.Rotation = 0.0f;
        obj->SetTransform(res);
        obj->SetLayer(pixel_engine::ELayer::Obstacles);
        obj->SetTexture("assets/sprites/A.png");

        pixel_engine::PERenderQueue::Instance().AddSprite(obj.get());
        m_objects.push_back(std::move(obj));
    }

    m_anim = std::make_unique<pixel_engine::TileAnim>(m_object.get());

    for (int i = 0; i < 40; i++)
    {
        std::string prefix = "female_idle0";
        if (i <= 9) prefix += "0" + std::to_string(i);
        else prefix += std::to_string(i);
        prefix += ".png";
        m_anim->AddFrame("assets/sprites/female/" + prefix);
    }
    m_anim->Build();
}

void pixel_game::Application::Tick(float deltaTime)
{
    m_time += deltaTime;
    const float dRot = kRotSpeed * deltaTime;

    auto cam = pixel_engine::PERenderQueue::Instance().GetCamera();
    
    pixel_engine::PFE_WORLD_SPACE_DESC desc{};
    desc.pCamera = cam;
    desc.Origin  = cam->WorldToCamera({ 0.0f, 0.0f }, 32);
    desc.X1      = cam->WorldToCamera({ 1.0f, 0.0f }, 32);
    desc.Y1      = cam->WorldToCamera({ 0.0f, 1.0f }, 32);

    m_anim->OnFrameBegin(deltaTime);
    m_object->Update(deltaTime, desc);

    for (auto& obj: m_objects)
    {
        obj->Update(deltaTime, desc);
    }
    m_anim->OnFrameEnd();
}

void pixel_game::Application::Release()
{
}
