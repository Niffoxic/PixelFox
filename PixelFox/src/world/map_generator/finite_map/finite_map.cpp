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
    m_ppszStones.push_back(data);
    data.texture_base = "assets/stones/1.png";
    m_ppszStones.push_back(data);
    data.texture_base = "assets/stones/2.png";
    m_ppszStones.push_back(data);
    data.texture_base = "assets/stones/3.png";
    m_ppszStones.push_back(data);
    data.texture_base = "assets/stones/0.png";
    m_ppszStones.push_back(data);

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
            spawnDesc.SpawnMaxCount  = 10;
            spawnDesc.SpawnRampTime  = 10.f;
            spawnDesc.SpawnStartTime = 5.f;
            m_pEnemySpawner->StopAtLimit(true);
            m_pEnemySpawner->Initialize(spawnDesc);
        }
    }

    if (desc.LoadScreen.pLoadDescription)
    {
        desc.LoadScreen.pLoadDescription->SetText("Loading complete.");
    }
}

void pixel_game::FiniteMap::Update(float deltaTime)
{
    if (deltaTime > 1.f) return;
    HandleInput(deltaTime);

    for (const auto& [type, vec] : m_ppObsticle)
    {
        const int live = m_liveCount[type];
        for (int i = 0; i < live; ++i)
            if (vec[i]) vec[i]->Update(deltaTime);
    }

    if (m_pPlayerCharacter)
    {
        m_pPlayerCharacter->Update(deltaTime);

        if (m_pKeyboard)
            m_pPlayerCharacter->HandleInput(m_pKeyboard, deltaTime);

        RestrictPlayer();
    }

    if (m_pEnemySpawner)
        m_pEnemySpawner->Update(deltaTime);

    m_nElapsedTime += deltaTime;
    UpdateMapGUI(deltaTime);
    MapCycle();
}

void pixel_game::FiniteMap::Release()
{
    for (const auto& [type, vec] : m_ppObsticle)
    {
        for (auto& obj : vec)
        {
            if (obj) obj->Hide();
        }
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
        pixel_engine::PERenderQueue::Instance().RemoveFont(m_level.get());
    }
}

void pixel_game::FiniteMap::Start()
{
    AttachCamera();
    m_pPlayerCharacter->Draw();

    for (const auto& [type, vec] : m_ppObsticle)
    {
        const int live = m_liveCount[type];
        for (int i = 0; i < live; ++i)
        {
            if (vec[i]) vec[i]->Draw();
        }
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
    m_bActive = false;
    m_bPaused = false;
    m_bComplete = false;
    m_nElapsedTime = 0.0f;

    if (m_pEnemySpawner)
        m_pEnemySpawner->Restart();

    m_nElapsedTime = 1;
    AttachCamera();
}


void pixel_game::FiniteMap::UnLoad()
{
    DettachCamera();
    if (m_pPlayerCharacter)
        m_pPlayerCharacter->Hide();

    if (m_pEnemySpawner) m_pEnemySpawner->Hide();

    for (const auto& [type, vec] : m_ppObsticle)
    {
        for (auto& obj : vec)
        {
            if (obj) obj->Hide();
        }
    }

    pixel_engine::PERenderQueue::Instance().RemoveFont(m_timer.get());
    pixel_engine::PERenderQueue::Instance().RemoveFont(m_level.get());
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildMapObjects(LOAD_SCREEN_DETAILS details)
{

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
        pixel_engine::logger::error(
            "FiniteMap::LoadMap - failed to open '{}'",
            levelPath);
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

    BeginReuseFrame_();

    BuildMapObjects(dummy);
    SetPlayerSpawnPosition();
    AttachCamera();
    BuildMapGUI(dummy);

    if (m_pEnemySpawner) m_pEnemySpawner->Restart();

    if (m_pPlayerCharacter) m_pPlayerCharacter->Draw();

    for (const auto& [type, vec] : m_ppObsticle)
    {
        const int live = m_liveCount[type];
        for (int i = 0; i < live; ++i)
            if (vec[i]) vec[i]->Draw();
    }

    HideUnused_(); 
}

_Use_decl_annotations_
bool pixel_game::FiniteMap::SpawnObjectFromFileDataVec
(const fox::vector<FileData>& pool,
    const FVector2D& gridPos,
    const FVector2D& scale,
    char typeKey)
{
    if (pool.empty()) return false;

    const int idx = GetRandomNumber(0, static_cast<int>(pool.size()) - 1);
    const auto& data = pool[idx];

    INIT_OBSTICLE_DESC desc{};
    desc.szName = data.file_name;
    desc.baseTexture = data.texture_base;
    desc.obsticleSprites = data.texture_sprite;
    desc.scale = scale;
    desc.position = m_Bounds.min + gridPos;

    return AcquireObsticle_(typeKey, desc) != nullptr;
}

void pixel_game::FiniteMap::BuildTrees(LOAD_SCREEN_DETAILS details)
{
    const std::string level = LoadMap();
    if (level.empty() || m_ppszTress.empty()) return;

    if (details.pLoadDescription) details.pLoadDescription->SetText("Placing Trees from file...");

    int count = 0;
    ForEachLevelCell(level, [&](int gx, int gy, char ch)
        {
            if (ch != 't') return;
            if (SpawnObjectFromFileDataVec(m_ppszTress, { float(gx), float(gy) }, { 2.f, 2.f }, 't'))
                ++count;
        });

    pixel_engine::logger::debug("FiniteMap::BuildTrees - Placed {} trees from file", count);
}

void pixel_game::FiniteMap::BuildStones(LOAD_SCREEN_DETAILS details)
{
    const std::string level = LoadMap();
    if (level.empty() || m_ppszStones.empty()) return;

    if (details.pLoadDescription) details.pLoadDescription->SetText("Placing Stones from file...");

    int count = 0;
    ForEachLevelCell(level, [&](int gx, int gy, char ch)
        {
            if (ch != 's') return;
            if (SpawnObjectFromFileDataVec(m_ppszStones, { float(gx), float(gy) }, { 1.f, 1.f }, 's'))
                ++count;
        });

    pixel_engine::logger::debug("FiniteMap::BuildStones - Placed {} stones from file", count);
}

void pixel_game::FiniteMap::BuildWaters(LOAD_SCREEN_DETAILS details)
{
    const std::string level = LoadMap();
    if (level.empty() || m_ppszWater.empty()) return;

    if (details.pLoadDescription) details.pLoadDescription->SetText("Placing Waters from file...");

    int count = 0;
    ForEachLevelCell(level, [&](int gx, int gy, char ch)
        {
            if (ch != 'w' && ch != 'W') return;
            if (SpawnObjectFromFileDataVec(m_ppszWater, { float(gx), float(gy) }, { 1.f, 1.f }, 'w'))
                ++count;
        });

    pixel_engine::logger::debug("FiniteMap::BuildWaters - Placed {} waters from file", count);
}

void pixel_game::FiniteMap::BuildGround(LOAD_SCREEN_DETAILS /*details*/)
{
    if (m_ppszGround.empty()) return;

    const FVector2D bigScale{ 64.f, 64.f };
    const auto& data = m_ppszGround.front();

    const FVector2D positions[] =
    {
        {  0.f,   0.f },
        { -64.f,  0.f }, { 64.f,  0.f }, { 0.f,  64.f }, { 0.f, -64.f },
        { -64.f, 64.f }, { 64.f, 64.f }, { -64.f, -64.f }, { 64.f, -64.f }
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

        if (auto* o = AcquireObsticle_('g', desc))
        {
            if (auto* col = o->GetCollider())
            {
                col->SetScale(desc.scale);
                col->SetColliderType(pixel_engine::ColliderType::Trigger);
            }
            if (auto* sprite = o->GetSpirte())
                sprite->SetLayer(pixel_engine::ELayer::Background);

            ++placed;
        }
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

    UnLoad();

    if (m_nCurrentLevel >= m_nMaxLevel)
    {
        m_nCurrentLevel = 1;
        if (m_pEnemySpawner) m_pEnemySpawner->Restart();
        if (m_OnMapComplete) m_OnMapComplete();
        return;
    }

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

void pixel_game::FiniteMap::HandleInput(float deltaTime)
{
    if (!m_pKeyboard) return;

    //~ is it on cool down
    if (m_nInputTimer > 0.0f)
    {
        m_nInputTimer -= deltaTime;
        if (m_nInputTimer > 0.0f) return;
    }

    if (m_pKeyboard)
    {
        if (m_pKeyboard->WasChordPressed('S', pixel_engine::PEKeyboardMode::Ctrl))
        {
            SaveState();
            m_nInputTimer = m_nInputDelay;
        }

        if (m_pKeyboard->WasChordPressed('L', pixel_engine::PEKeyboardMode::Ctrl))
        {
            LoadState();
            m_nInputTimer = m_nInputDelay;
        }
    }
}

void pixel_game::FiniteMap::LoadState()
{
    pixel_engine::logger::error("Loading State!");

    if (!pixel_engine::PEFileSystem::IsPathExists(m_szSavedPath))
    {
        pixel_engine::logger::error("Saved Path Does not exists!");
        return;
    }
    m_foxLoader.Load(m_szSavedPath);

    // Defaults
    int   savedLevel = m_nCurrentLevel;
    float savedTime = 0.0f;

    if (m_foxLoader.Contains("CurrentLevel"))
        savedLevel = m_foxLoader["CurrentLevel"].AsInt();

    if (m_foxLoader.Contains("Time"))
        savedTime = m_foxLoader["Time"].AsFloat();

    // Clamp and apply
    savedLevel = std::max(1, std::min(savedLevel, m_nMaxLevel));
    m_nCurrentLevel = savedLevel;
    m_nElapsedTime = std::max(0.f, std::min(savedTime, (float)m_nMapDuration));

    // Rebuild the world immediately for this level
    RebuildLevel_();

    // Update GUI
    if (m_level) m_level->SetText("Level: " 
        + std::to_string(m_nCurrentLevel));
    if (m_timer) m_timer->SetText("Time Left: "
        + std::to_string(m_nMapDuration - (int)m_nElapsedTime));

    // Ensure fonts are visible again
    if (m_timer) pixel_engine::PERenderQueue::Instance().AddFont(m_timer.get());
    if (m_level) pixel_engine::PERenderQueue::Instance().AddFont(m_level.get());

    // Player
    if (m_foxLoader.Contains("Player") && m_pPlayerCharacter)
    {
        const auto& player = m_foxLoader["Player"];
        const float px = player.Contains("PosX") ? player["PosX"].AsFloat() : 0.0f;
        const float py = player.Contains("PosY") ? player["PosY"].AsFloat() : 0.0f;
        const float health = player.Contains("Health") ? player["Health"].AsFloat() : m_pPlayerCharacter->GetPlayerHeath();

        if (auto* body = m_pPlayerCharacter->GetPlayerBody())
            body->SetPosition({ px, py });
        m_pPlayerCharacter->SetHealth(health);
    }

    // Enemies
    if (m_pEnemySpawner && m_foxLoader.Contains("Enemies"))
    {
        m_pEnemySpawner->Restart();
        m_pEnemySpawner->LoadState(m_foxLoader["Enemies"]);
    }

    pixel_engine::logger::debug(
        "LoadState: level={}, elapsed={}",
        m_nCurrentLevel, m_nElapsedTime);
}

void pixel_game::FiniteMap::SaveState()
{
    m_foxLoader.Clear();

    m_foxLoader.GetOrCreate("CurrentLevel")
        .SetValue(std::to_string(m_nCurrentLevel));
    m_foxLoader.GetOrCreate("Time")
        .SetValue(std::to_string(m_nElapsedTime));

    if (auto* body = m_pPlayerCharacter->GetPlayerBody())
    {
        const FVector2D pos = body->GetPosition();

        auto& player = m_foxLoader.GetOrCreate("Player");
        player.GetOrCreate("PosX").SetValue(std::to_string(pos.x));
        player.GetOrCreate("PosY").SetValue(std::to_string(pos.y));
        player.GetOrCreate("Health").SetValue(std::to_string(m_pPlayerCharacter->GetPlayerHeath()));
    }

    if (m_pEnemySpawner)
    {
        m_pEnemySpawner->SaveState(m_foxLoader.GetOrCreate("Enemies"));
    }
    m_foxLoader.Save(m_szSavedPath);
    pixel_engine::logger::debug("SaveState: saved to '{}'", m_szSavedPath);
}


void pixel_game::FiniteMap::BeginReuseFrame_()
{
    for (const auto& kv : m_ppObsticle) m_liveCount[kv.first] = 0;
}

void pixel_game::FiniteMap::HideUnused_()
{
    for (const auto& [type, vec] : m_ppObsticle)
    {
        const int live = m_liveCount[type];
        for (int i = live; i < static_cast<int>(vec.size()); ++i)
            if (vec[i]) vec[i]->Hide();
    }
}

Obsticle* pixel_game::FiniteMap::AcquireObsticle_(char type, const INIT_OBSTICLE_DESC& desc)
{
    auto& vec = m_ppObsticle[type];
    int& live = m_liveCount[type]; //~ default init 

    if (live < static_cast<int>(vec.size()))
    {
        Obsticle* o = vec[live].get();
        if (o)
        {
            o->Draw();

            if (auto* spr = o->GetSpirte()) { spr->SetPosition(desc.position); spr->SetScale(desc.scale); }
            else { o->SetPosition(desc.position); }

            if (auto* col = o->GetCollider()) col->SetScale(desc.scale);
        }
        ++live;
        return o;
    }
    else
    {
        auto obj = std::make_unique<Obsticle>();
        if (!obj->Init(desc)) return nullptr;

        if (auto* spr = obj->GetSpirte()) { spr->SetPosition(desc.position); spr->SetScale(desc.scale); }
        if (auto* col = obj->GetCollider()) col->SetScale(desc.scale);

        Obsticle* raw = obj.get();
        vec.push_back(std::move(obj));
        ++live;
        return raw;
    }
}
