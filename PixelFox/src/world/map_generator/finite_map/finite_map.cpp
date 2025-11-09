#include "finite_map.h"
#include "pixel_engine/utilities/logger/logger.h"

#include <random>
#include <fstream>
#include <cctype> 

using namespace pixel_game;

pixel_game::FiniteMap::FiniteMap()
{
    //~ add tree data
    FileData data{};
    data.file_name = "test_tree";
    data.texture_base = "assets/trees/0.png";
    data.texture_sprite = "";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/trees/1.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/trees/2.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/trees/3.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/trees/4.png";
    m_ppszTress.push_back(data);

    //~ add stone data
    data.texture_base = "assets/stones/0.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/stones/1.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/stones/2.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/stones/3.png";
    m_ppszTress.push_back(data);
    data.texture_base = "assets/stones/2.png";
    m_ppszTress.push_back(data);

    //~ add water data
    data.texture_base = "assets/water/0.png";
    m_ppszWater.push_back(data);
    data.texture_base = "assets/water/1.png";
    m_ppszWater.push_back(data);
    data.texture_base = "assets/water/2.png";
    m_ppszWater.push_back(data);
    data.texture_base = "assets/water/3.png";
    m_ppszWater.push_back(data);
    data.texture_base = "assets/water/4.png";
    m_ppszWater.push_back(data);
    data.texture_base = "assets/water/5.png";
    m_ppszWater.push_back(data);

    //~ add background
    data.texture_base = "assets/ground/0.png";
    m_ppszGround.push_back(data);
    data.texture_base = "assets/ground/1.png";
    m_ppszGround.push_back(data);
    data.texture_base = "assets/ground/2.png";
    m_ppszGround.push_back(data);
    data.texture_base = "assets/ground/3.png";
    m_ppszGround.push_back(data);
    data.texture_base = "assets/ground/4.png";
    m_ppszGround.push_back(data);
}

_Use_decl_annotations_
void pixel_game::FiniteMap::Initialize(const MAP_INIT_DESC& desc)
{
    if (m_bInitialized) return;

    m_bInitialized = true;

    m_pKeyboard         = desc.pKeyboard;
    m_pPlayerCharacter  = desc.pPlayerCharacter;
    m_pEnemySpawner     = desc.pEnemySpawner;
    m_nMapDuration      = desc.MapDuration;
    m_bUseBounds        = desc.UseBounds;
    m_Bounds            = desc.Bounds;
    m_eType             = desc.Type;
    m_OnMapComplete     = desc.OnMapComplete;

    //~ Reset runtime state
    m_bActive      = false;
    m_bPaused      = false;
    m_bComplete    = false;
    m_nElapsedTime = 0.0f;

    pixel_engine::logger::debug("FiniteMap::Initialize - Duration: {}s", m_nMapDuration);

    //~ Prepare loading UI
    if (desc.LoadScreen.pLoadTitle != nullptr)
    {
        desc.LoadScreen.pLoadTitle->SetText("Loading Map...");
    }
    if (desc.LoadScreen.pLoadDescription != nullptr)
    {
        desc.LoadScreen.pLoadDescription->SetText("Preparing world...");
    }

    BuildMapObjects(desc.LoadScreen);

    if (desc.LoadScreen.pLoadTitle != nullptr)
    {
        desc.LoadScreen.pLoadTitle->SetText("Building Player...");
    }
    if (desc.LoadScreen.pLoadDescription != nullptr)
    {
        desc.LoadScreen.pLoadDescription->SetText("Player is being configured...");
    }

    SetPlayerSpawnPosition();
    AttachCamera();
    BuildMapGUI(desc.LoadScreen);

    if (m_pEnemySpawner)
    {
        if (!m_pEnemySpawner->IsInitialized())
        {
            PG_SPAWN_DESC spawnDesc{};
            spawnDesc.pLoadDescription = desc.LoadScreen.pLoadDescription;
            spawnDesc.pLoadTitle = desc.LoadScreen.pLoadTitle;
            spawnDesc.SpawnMaxCount = 200;
            spawnDesc.SpawnRampTime = 110.f;
            spawnDesc.SpawnStartTime = 5.f;
            m_pEnemySpawner->Initialize(spawnDesc);
        }
    }

    if (desc.LoadScreen.pLoadDescription)
    {
        desc.LoadScreen.pLoadDescription->SetText("Loading complete.");
    }
}

_Use_decl_annotations_
void pixel_game::FiniteMap::Update(float deltaTime)
{
    if (deltaTime > 1.f) return;
    for (auto& obj : m_ppObsticle)
    {
        obj->Update(deltaTime);
    }

    if (m_pPlayerCharacter)
    {
        m_pPlayerCharacter->Update(deltaTime);

        if (m_pKeyboard)
        {
            m_pPlayerCharacter->HandleInput(m_pKeyboard, deltaTime);
        }
        RestrictPlayer();
    }

    if (m_pEnemySpawner)
    {
        m_pEnemySpawner->Update(deltaTime);
    }

    m_nElapsedTime += deltaTime;
    UpdateMapGUI(deltaTime);
    MapCycle();
}

void pixel_game::FiniteMap::Release()
{
    for (auto& obj : m_ppObsticle) 
    {
        obj->Hide();
        obj->Release();
    }

    if (m_pPlayerCharacter)
    {
        m_pPlayerCharacter->Hide();
        m_pPlayerCharacter->UnloadFromQueue();
    }

    if (m_timer)
    {
        pixel_engine::PERenderQueue::Instance().RemoveFont(m_timer.get());
    }
    if (m_level)
    {
        pixel_engine::PERenderQueue::Instance().AddFont(m_level.get());
    }
}

void pixel_game::FiniteMap::Start()
{
    AttachCamera();
    m_pPlayerCharacter->Draw();
    for (auto& obj : m_ppObsticle) 
    {
        obj->Draw();
    }
    m_nCurrentLevel = 1;
    pixel_engine::PERenderQueue::Instance().AddFont(m_timer.get());
    pixel_engine::PERenderQueue::Instance().AddFont(m_level.get());
}

void pixel_game::FiniteMap::Pause()
{
}

void pixel_game::FiniteMap::Resume()
{
}

void pixel_game::FiniteMap::Restart()
{
    //~ Reset runtime state
    m_bActive      = false;
    m_bPaused      = false;
    m_bComplete    = false;
    m_nElapsedTime = 0.0f;
    m_nCurrentLevel = 1;

    if (m_pEnemySpawner)
    {
        m_pEnemySpawner->Restart();
    }
    AttachCamera();
}

void pixel_game::FiniteMap::UnLoad()
{
    DettachCamera();
    m_pPlayerCharacter->Hide();

    if (m_pEnemySpawner) m_pEnemySpawner->Hide();

    for (auto& obj : m_ppObsticle)
    {
        obj->Hide();
    }
    pixel_engine::PERenderQueue::Instance().RemoveFont(m_timer.get());
    pixel_engine::PERenderQueue::Instance().RemoveFont(m_level.get());
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildMapObjects(LOAD_SCREEN_DETAILS details)
{

    m_ppObsticle.clear();

    if (details.pLoadTitle)
        details.pLoadTitle->SetText("Building Map|Trees|");
    BuildTrees(details);

    if (details.pLoadTitle)
        details.pLoadTitle->SetText("Building Map|Waters|");
    BuildWaters(details);

    if (details.pLoadTitle)
        details.pLoadTitle->SetText("Building Map|Stones|");
    BuildStones(details);

    if (details.pLoadTitle)
        details.pLoadTitle->SetText("Building Map|Ground|");
    BuildGround(details);

    pixel_engine::logger::debug(
        "FiniteMap::BuildMapObjects completed — Obsticles: {}",
        m_ppObsticle.size());
}

std::string pixel_game::FiniteMap::LoadMap()
{
    const std::string levelPath =
        "level/finiteLevel_" + std::to_string(m_nCurrentLevel) + ".txt";

    std::ifstream in(levelPath, std::ios::in);
    if (!in)
    {
        pixel_engine::logger::error("FiniteMap::LoadMap - failed to open '{}'", levelPath);
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void pixel_game::FiniteMap::AdvanceLevel_()
{
    m_nCurrentLevel = (m_nCurrentLevel % m_nMaxLevel) + 1;
    std::string message = "Level: " + std::to_string(m_nCurrentLevel);
    if (m_level) m_level->SetText(message);
}

void pixel_game::FiniteMap::RebuildLevel_()
{
    LOAD_SCREEN_DETAILS dummy{};

    m_ppObsticle.clear();
    
    BuildMapObjects(dummy);
    SetPlayerSpawnPosition();
    AttachCamera();
    BuildMapGUI(dummy);

    if (m_pEnemySpawner) m_pEnemySpawner->Restart();

    if (m_pPlayerCharacter) m_pPlayerCharacter->Draw();
    for (auto& obj : m_ppObsticle) if (obj) obj->Draw();
}

_Use_decl_annotations_
bool pixel_game::FiniteMap::SpawnObjectFromFileDataVec
(const fox::vector<FileData>& pool,
    const FVector2D& gridPos,
    const FVector2D& scale)
{
    if (pool.empty()) return false;

    const int idx = GetRandomNumber(0, static_cast<int>(pool.size()) - 1);
    const auto& data = pool[idx];

    INIT_OBSTICLE_DESC desc{};
    desc.szName          = data.file_name;
    desc.baseTexture     = data.texture_base;
    desc.obsticleSprites = data.texture_sprite;
    desc.scale           = scale;
    desc.position        = m_Bounds.min + gridPos;

    auto obst = std::make_unique<Obsticle>();
    if (!obst->Init(desc)) return false;

    if (auto* col = obst->GetCollider())
    {
        FVector2D scale = desc.scale;
        col->SetScale(scale);
    }
    m_ppObsticle.push_back(std::move(obst));
    return true;
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildTrees(LOAD_SCREEN_DETAILS details)
{
    const std::string level = LoadMap();
    if (level.empty() || m_ppszTress.empty())
        return;

    if (details.pLoadDescription)
        details.pLoadDescription->SetText("Placing Trees from file...");

    int count = 0;
    ForEachLevelCell(level, [&](int gx, int gy, char ch)
    {
        if (ch != 't') return;
        if (SpawnObjectFromFileDataVec(m_ppszTress, { float(gx), float(gy) }, { 2.f, 2.f }))
            ++count;
    });

    pixel_engine::logger::debug("FiniteMap::BuildTrees - Placed {} trees from file", count);
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildStones(LOAD_SCREEN_DETAILS details)
{
    const std::string level = LoadMap();
    if (level.empty() || m_ppszStones.empty())
        return;

    if (details.pLoadDescription)
        details.pLoadDescription->SetText("Placing Stones from file...");

    int count = 0;
    ForEachLevelCell(level, [&](int gx, int gy, char ch)
    {
        if (ch != 's') return;
        if (SpawnObjectFromFileDataVec(m_ppszStones, { float(gx), float(gy) }, { 1.f, 1.f }))
            ++count;
    });

    pixel_engine::logger::debug("FiniteMap::BuildStones - Placed {} stones from file", count);
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildWaters(LOAD_SCREEN_DETAILS details)
{
    const std::string level = LoadMap();
    if (level.empty() || m_ppszWater.empty())
        return;

    if (details.pLoadDescription)
        details.pLoadDescription->SetText("Placing Waters from file...");

    int count = 0;
    ForEachLevelCell(level, [&](int gx, int gy, char ch)
    {
        if (ch != 'w' && ch != 'W') return;
        if (SpawnObjectFromFileDataVec(m_ppszWater,
            { float(gx), float(gy) }, { 1.f, 1.f }))
            ++count;
    });

    pixel_engine::logger::debug("FiniteMap::BuildWaters - Placed {} waters from file", count);
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildGround(LOAD_SCREEN_DETAILS details)
{
    if (m_ppszGround.empty())
        return;

    const FVector2D bigScale{ 64.f, 64.f };
    const auto& data = m_ppszGround.front();

    const FVector2D positions[] =
    {
        // Center
        {  0.f,   0.f },

        { -64.f,   0.f }, // left
        {  64.f,   0.f }, // right
        {  0.f,  64.f },  // up
        {  0.f, -64.f },  // down

        { -64.f,  64.f }, // up-left
        {  64.f,  64.f }, // up-right
        { -64.f, -64.f }, // down-left
        {  64.f, -64.f }  // down-right
    };

    int placed = 0;

    for (const auto& pos : positions)
    {
        INIT_OBSTICLE_DESC desc{};
        desc.szName = data.file_name;
        desc.baseTexture = data.texture_base;
        desc.obsticleSprites = data.texture_sprite;
        desc.scale = bigScale;
        desc.position = pos;

        auto obst = std::make_unique<Obsticle>();
        if (!obst->Init(desc))
            continue;

        if (auto* col = obst->GetCollider())
        {
            col->SetScale(desc.scale);
            col->SetColliderType(pixel_engine::ColliderType::Trigger);
        }

        if (auto* sprite = obst->GetSpirte())
            sprite->SetLayer(pixel_engine::ELayer::Background);

        m_ppObsticle.push_back(std::move(obst));
        ++placed;
    }

    pixel_engine::logger::debug("FiniteMap::BuildGround - Placed {} ground tiles (center + 8 surround).", placed);
}

void pixel_game::FiniteMap::SetPlayerSpawnPosition()
{
    if (m_pPlayerCharacter)
    {
        if (!m_pPlayerCharacter->IsInitialized())
        {
            m_pPlayerCharacter->Initialize();
        }
        if (auto* body = m_pPlayerCharacter->GetPlayerBody())
        {
            body->SetPosition({ 0.f, 0.f });
        }
        
    }
}

void pixel_game::FiniteMap::AttachCamera()
{
    if (auto* cam = pixel_engine::PERenderQueue::Instance().GetCamera())
    {
        if (m_pPlayerCharacter && m_pPlayerCharacter->GetPlayerBody())
        {
            cam->FollowSprite(m_pPlayerCharacter->GetPlayerBody());
        }
    }
}

void pixel_game::FiniteMap::DettachCamera()
{
    if (auto* cam = pixel_engine::PERenderQueue::Instance().GetCamera())
    {
        if (m_pPlayerCharacter && m_pPlayerCharacter->GetPlayerBody())
        {
            cam->FollowSprite(nullptr);
            cam->SetPosition({ 0, 0 });
        }
    }
}

void pixel_game::FiniteMap::RestrictPlayer()
{
    if (!m_pPlayerCharacter)
        return;

    auto* body = m_pPlayerCharacter->GetPlayerBody();
    if (!body)
        return;

    FVector2D pos = body->GetPosition();
    FVector2D scale = body->GetScale();

    const float halfWidth = scale.x * 0.5f;
    const float halfHeight = scale.y * 0.5f;

    if (pos.x - halfWidth < m_Bounds.min.x)
        pos.x = m_Bounds.min.x + halfWidth;
    else if (pos.x + halfWidth > m_Bounds.max.x)
        pos.x = m_Bounds.max.x - halfWidth;

    if (pos.y - halfHeight < m_Bounds.min.y)
        pos.y = m_Bounds.min.y + halfHeight;
    else if (pos.y + halfHeight > m_Bounds.max.y)
        pos.y = m_Bounds.max.y - halfHeight;

    body->SetPosition(pos);
}

void pixel_game::FiniteMap::BuildMapGUI(LOAD_SCREEN_DETAILS details)
{
    if (!m_level)
    {
        m_level = std::make_unique<pixel_engine::PEFont>();
        m_level->SetPosition({ 400, 80 });
        m_level->SetPx(16);
        std::string message = "Level: 1";
        m_level->SetText(message);
    }
    if (!m_timer)
    {
        m_timer = std::make_unique<pixel_engine::PEFont>();
        m_timer->SetPosition({ 400, 50 });
        m_timer->SetText("Time Left:");
    }
}

void pixel_game::FiniteMap::UpdateMapGUI(float deltaTime)
{
    int time_left = m_nMapDuration - m_nElapsedTime;
    std::string message = "Time Left: " + std::to_string(time_left);
    
    if (m_timer) 
    {
        m_timer->SetText(message);
    }
}

int pixel_game::FiniteMap::GetRandomNumber(int min, int max)
{
    if (min > max) std::swap(min, max);

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(min, max);

    return dist(rng);
}

void pixel_game::FiniteMap::MapCycle()
{
    if (m_nElapsedTime < m_nMapDuration) return;


    if (m_nCurrentLevel >= m_nMaxLevel)
    {
        if (m_OnMapComplete) m_OnMapComplete();
        return;
    }

    UnLoad();
    DettachCamera();

    AdvanceLevel_();

    m_bActive       = false;
    m_bPaused       = false;
    m_bComplete     = false;
    m_nElapsedTime  = 0.0f;

    RebuildLevel_();

    if (m_timer) pixel_engine::PERenderQueue::Instance().AddFont(m_timer.get());
    if (m_level) pixel_engine::PERenderQueue::Instance().AddFont(m_level.get());
}

void pixel_game::FiniteMap::AllocateEnemy(LOAD_SCREEN_DETAILS details)
{
    if (m_pEnemySpawner)
    {
        PG_SPAWN_DESC desc{};
        desc.SpawnMaxCount    = 200;
        desc.SpawnRampTime    = 60.f;
        desc.SpawnStartTime   = 5.f;
        desc.pLoadDescription = details.pLoadDescription;
        desc.pLoadTitle       = details.pLoadTitle;

        if (!m_pEnemySpawner->IsInitialized())
        {
            m_pEnemySpawner->Initialize(desc);
        }
        else m_pEnemySpawner->Reset();
    }
}
