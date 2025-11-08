#include "finite_map.h"
#include "pixel_engine/utilities/logger/logger.h"

#include <random>

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
}

_Use_decl_annotations_
void pixel_game::FiniteMap::Initialize(const MAP_INIT_DESC& desc)
{
    m_bInitialized = true;

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
    SetPlayerSpawnPosition();

    if (desc.LoadScreen.pLoadDescription)
    {
        desc.LoadScreen.pLoadDescription->SetText("Loading complete.");
    }
}

_Use_decl_annotations_
void pixel_game::FiniteMap::Update(float deltaTime)
{
    for (auto& obj : m_ppObsticle)
    {
        obj->Update(deltaTime);
    }
}

void pixel_game::FiniteMap::Release()
{
    for (auto& obj : m_ppObsticle) 
    {
        obj->Hide();
        obj->Release();
    }
}

void pixel_game::FiniteMap::Start()
{
    for (auto& obj : m_ppObsticle) 
    {
        obj->Draw();
    }
}

void pixel_game::FiniteMap::Pause()
{
}

void pixel_game::FiniteMap::Resume()
{
}

void pixel_game::FiniteMap::Restart()
{
}

void pixel_game::FiniteMap::UnLoad()
{
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

    pixel_engine::logger::debug(
        "FiniteMap::BuildMapObjects completed — Obsticles: {}",
        m_ppObsticle.size());
}

void pixel_game::FiniteMap::SetPlayerSpawnPosition()
{
    if (m_pPlayerCharacter)
    {
        m_pPlayerCharacter->GetPlayerBody()->SetPosition({ 0.f, 0.f });
    }
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildTrees(LOAD_SCREEN_DETAILS details)
{
    if (m_ppszTress.empty())
        return;

    constexpr int SpawnCount = 200;
    int totalSpawned = 0;

    if (details.pLoadDescription)
        details.pLoadDescription->SetText("Spawning Random Trees...");

    while (totalSpawned < SpawnCount)
    {
        const int index = GetRandomNumber(0, static_cast<int>(m_ppszTress.size()) - 1);
        const auto& data = m_ppszTress[index];

        int clusterCount = GetRandomNumber(1, 3);
        FVector2D spawnPosition = GetRandomPositionInBound();

        for (int i = 0; i < clusterCount && totalSpawned < SpawnCount; ++i)
        {
            INIT_OBSTICLE_DESC desc{};
            desc.szName = data.file_name;
            desc.baseTexture = data.texture_base;
            desc.obsticleSprites = data.texture_sprite;
            desc.scale = { 3.f, 3.f };
            desc.position = spawnPosition;

            auto obst = std::make_unique<Obsticle>();
            if (obst->Init(desc))
            {
                if (auto* col = obst->GetCollider())
                    col->SetScale(desc.scale);

                m_ppObsticle.push_back(std::move(obst));
                ++totalSpawned;
            }
            spawnPosition.x += 2.f;
        }
    }

    pixel_engine::logger::debug(
        "FiniteMap::BuildTrees - Spawned {} random trees using {} types",
        totalSpawned,
        m_ppszTress.size());
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildStones(LOAD_SCREEN_DETAILS details)
{
    if (m_ppszStones.empty())
        return;

    constexpr int SpawnCount = 150;
    int totalSpawned = 0;

    if (details.pLoadDescription)
        details.pLoadDescription->SetText("Spawning Random Stones...");

    while (totalSpawned < SpawnCount)
    {
        const int index = GetRandomNumber(0, static_cast<int>(m_ppszStones.size()) - 1);
        const auto& data = m_ppszStones[index];

        // small side by side
        const int clusterCount = GetRandomNumber(1, 3);

        FVector2D spawnPosition = GetRandomPositionInBound();

        for (int i = 0; i < clusterCount && totalSpawned < SpawnCount; ++i)
        {
            INIT_OBSTICLE_DESC desc{};
            desc.szName = data.file_name;
            desc.baseTexture = data.texture_base;
            desc.obsticleSprites = data.texture_sprite;
            desc.scale = { 1.f, 1.f };
            desc.position = spawnPosition;

            auto obst = std::make_unique<Obsticle>();
            if (obst->Init(desc))
            {
                if (auto* col = obst->GetCollider())
                    col->SetScale(desc.scale);

                m_ppObsticle.push_back(std::move(obst));
                ++totalSpawned;
            }
            spawnPosition.x += 1.f;
        }
    }

    pixel_engine::logger::debug(
        "FiniteMap::BuildStones - Spawned {} random stones using {} types",
        totalSpawned,
        m_ppszStones.size());
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildWaters(LOAD_SCREEN_DETAILS details)
{
    if (m_ppszWater.empty())
        return;

    constexpr int SpawnCount = 150;
    int totalSpawned = 0;

    if (details.pLoadDescription)
        details.pLoadDescription->SetText("Spawning Random Waters...");

    while (totalSpawned < SpawnCount)
    {
        const int idx = GetRandomNumber(0, static_cast<int>(m_ppszWater.size()) - 1);
        const auto& data = m_ppszWater[idx];

        const int style = GetRandomNumber(0, 2);
        fox::vector<FVector2D> offsets;

        if (style == 0)
        {
            const int n = GetRandomNumber(4, 10);
            offsets = MakeWaterLine(n);
        }
        else if (style == 1)
        {
            const int w = GetRandomNumber(2, 5);
            const int h = GetRandomNumber(2, 4);
            offsets = MakeWaterBlock(w, h);
        }
        else
        {
            const int n = GetRandomNumber(5, 12);
            offsets = MakeWaterBlob(n);
        }

        const FVector2D start = GetRandomPositionInBound();
        BuildWaterCluster(data, start, offsets, totalSpawned, SpawnCount, details);
    }

    pixel_engine::logger::debug(
        "FiniteMap::BuildWaters - Spawned {} random waters using {} types",
        totalSpawned,
        m_ppszWater.size());
}

_Use_decl_annotations_
bool pixel_game::FiniteMap::InBounds(const FVector2D& p) const
{
    return (p.x >= m_Bounds.min.x && p.x <= m_Bounds.max.x &&
        p.y >= m_Bounds.min.y && p.y <= m_Bounds.max.y);
}

_Use_decl_annotations_
bool pixel_game::FiniteMap::TrySpawnWaterTile(const FileData& data, const FVector2D& pos)
{
    if (!InBounds(pos)) return false;

    INIT_OBSTICLE_DESC desc{};
    desc.szName = data.file_name;
    desc.baseTexture = data.texture_base;
    desc.obsticleSprites = data.texture_sprite;
    desc.scale = { 2.f, 2.f };
    desc.position = pos;

    auto obst = std::make_unique<Obsticle>();
    if (!obst->Init(desc)) return false;

    if (auto* col = obst->GetCollider())
        col->SetScale(desc.scale);

    m_ppObsticle.push_back(std::move(obst));
    return true;
}

_Use_decl_annotations_
fox::vector<FVector2D> pixel_game::FiniteMap::MakeWaterLine(int n) const
{
    fox::vector<FVector2D> out; out.reserve(n);
    for (int i = 0; i < n; ++i) out.push_back({ float(2 * i), 0.f });
    return out;
}

_Use_decl_annotations_
fox::vector<FVector2D> pixel_game::FiniteMap::MakeWaterBlock(int w, int h) const
{
    fox::vector<FVector2D> out; out.reserve(w * h);
    for (int j = 0; j < h; ++j)
        for (int i = 0; i < w; ++i)
            out.push_back({ float(2 * i), float(2 * j) });
    return out;
}

_Use_decl_annotations_
fox::vector<FVector2D> pixel_game::FiniteMap::MakeWaterBlob(int n)
{
    fox::vector<FVector2D> pts;
    pts.push_back({ 0.f, 0.f });
    for (int k = 1; k < n; ++k)
    {
        const int base = GetRandomNumber(0, static_cast<int>(pts.size()) - 1);
        FVector2D p = pts[base];
        switch (GetRandomNumber(0, 3))
        {
        case 0: p.x += 2.f; break;
        case 1: p.x -= 2.f; break;
        case 2: p.y += 2.f; break;
        default:p.y -= 2.f; break;
        }
        bool dup = false;
        for (const auto& q : pts) { if (q.x == p.x && q.y == p.y) { dup = true; break; } }
        if (!dup) pts.push_back(p);
    }
    return pts;
}

_Use_decl_annotations_
void pixel_game::FiniteMap::BuildWaterCluster(
    const FileData& data,
    const FVector2D& start,
    const fox::vector<FVector2D>& offsets,
    int& totalSpawned,
    const int spawnLimit,
    LOAD_SCREEN_DETAILS& details)
{
    for (const auto& off : offsets)
    {
        if (totalSpawned >= spawnLimit) break;
        FVector2D pos{ start.x + off.x, start.y + off.y };
        if (TrySpawnWaterTile(data, pos))
        {
            ++totalSpawned;
            if (details.pLoadDescription && (totalSpawned % 10 == 0))
                details.pLoadDescription->SetText(std::format("Spawning Random Waters... {}", totalSpawned));
        }
    }
}

void pixel_game::FiniteMap::BuildMapGUI(LOAD_SCREEN_DETAILS details)
{
}

FVector2D pixel_game::FiniteMap::GetRandomPositionInBound()
{
    if (m_Bounds.max.x <= m_Bounds.min.x || m_Bounds.max.y <= m_Bounds.min.y)
        return FVector2D{ 0.f, 0.f };

    static std::mt19937 rng{ std::random_device{}() };

    std::uniform_real_distribution<float> distX(m_Bounds.min.x, m_Bounds.max.x);
    std::uniform_real_distribution<float> distY(m_Bounds.min.y, m_Bounds.max.y);

    float x = std::floor(distX(rng));
    float y = std::floor(distY(rng));

    return FVector2D{ x, y };
}

int pixel_game::FiniteMap::GetRandomNumber(int min, int max)
{
    if (min > max) std::swap(min, max);

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(min, max);

    return dist(rng);
}
