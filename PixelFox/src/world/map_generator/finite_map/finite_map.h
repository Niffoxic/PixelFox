#pragma once

#include "world/map_generator/interface_map.h"
#include "obsticle/obsiticle.h"

#include "pixel_engine/utilities/fox_loader/fox_loader.h"

namespace pixel_game
{
	class FiniteMap final : public IMap
	{
		struct FileData
		{
			std::string file_name;
			std::string texture_sprite; // for anims
			std::string texture_base;
		};
	public:
		 FiniteMap();
		~FiniteMap() override = default;

		//~ IMap Implementation
		void Initialize(_In_ const MAP_INIT_DESC& desc) override;
		void Update(_In_ float deltaTime) override;
		void Release() override;

		void Start  () override;
		void Pause  () override;
		void Resume () override;
		void Restart() override;
		void UnLoad () override;

		_NODISCARD bool		  IsInitialized() const override { return m_bInitialized; }
		_NODISCARD bool		  IsActive	   () const override { return m_bActive; }
		_NODISCARD bool		  IsPaused	   () const override { return m_bPaused; }
		_NODISCARD bool		  IsComplete   () const override { return m_bComplete; }
		_NODISCARD float	  GetElapsed   () const override { return m_nElapsedTime; }
		_NODISCARD float	  GetRemaining () const override { return m_nMapDuration - m_nElapsedTime; }
		_NODISCARD bool		  HasBounds    () const override { return m_bUseBounds; }
		_NODISCARD MAP_BOUNDS GetBounds    () const override { return m_Bounds; }
		_NODISCARD EMapType   GetType      () const override { return m_eType; }

	private:
		void BuildMapObjects(_In_ LOAD_SCREEN_DETAILS details);
		std::string LoadMap();
		void AdvanceLevel_();
		void RebuildLevel_();

		template <typename Fn>
		void ForEachLevelCell(const std::string& level, Fn&& fn);

		bool SpawnObjectFromFileDataVec(
			_In_ const fox::vector<FileData>& pool,
			_In_ const FVector2D& gridPos,
			_In_ const FVector2D& scale,
			_In_ char typeKey);

		//~ build obsticles
		void BuildTrees(_In_ LOAD_SCREEN_DETAILS details);
		void BuildStones(_In_ LOAD_SCREEN_DETAILS details);
		void BuildWaters(_In_ LOAD_SCREEN_DETAILS details);
		void BuildGround(_In_ LOAD_SCREEN_DETAILS details);

		//~ player properties
		void SetPlayerSpawnPosition();
		void AttachCamera();
		void DettachCamera();
		void RestrictPlayer();
		
		//~ GUI
		void BuildMapGUI(LOAD_SCREEN_DETAILS details);
		void UpdateMapGUI(float deltaTime);
		int GetRandomNumber(int min, int max);
		void MapCycle();

		//~ initialize enemies
		void AllocateEnemy(LOAD_SCREEN_DETAILS details);

		//~ save and load
		void HandleInput(float deltaTime);
		void LoadState();
		void SaveState();

		//~ cache
		void BeginReuseFrame_();
		void HideUnused_();
		Obsticle* AcquireObsticle_(char type, const INIT_OBSTICLE_DESC& desc);

	private:
		float m_nInputDelay{ 0.2f };
		float m_nInputTimer{ 0.2f };
		const std::string m_szSavedPath{ "Saved/save.txt" }; //  I know its bad but no time
		pixel_engine::PEFoxLoader m_foxLoader{};

		int m_nMaxLevel	   { 4 };
		int m_nCurrentLevel{ 1 };

		fox::unordered_map<char, fox::vector<std::unique_ptr<Obsticle>>> m_ppObsticle{};
		fox::unordered_map<char, int> m_liveCount{};

		pixel_engine::PEKeyboardInputs* m_pKeyboard{ nullptr };
		PlayerCharacter* m_pPlayerCharacter{ nullptr };
		EnemySpawner*	 m_pEnemySpawner{ nullptr };

		bool   m_bInitialized{ false };
		bool   m_bActive  { false };
		bool   m_bPaused  { false };
		bool   m_bComplete{ false };
		bool   m_bBuilt{ false };

		bool   m_bUseBounds  { false };
		float  m_nMapDuration{ 0.0f };
		float  m_nElapsedTime{ 0.0f };

		EMapType   m_eType { EMapType::Finite };
		MAP_BOUNDS m_Bounds{};

		std::function<void()> m_OnMapComplete{};

		//~ GUI
		std::unique_ptr<pixel_engine::PEFont> m_timer{ nullptr };
		std::unique_ptr<pixel_engine::PEFont> m_level{ nullptr };

		//~ hard coded file location
		fox::vector<FileData> m_ppszTress {};
		fox::vector<FileData> m_ppszWater {};
		fox::vector<FileData> m_ppszStones{};
		fox::vector<FileData> m_ppszGround{};
	};

	template<typename Fn>
	inline void FiniteMap::ForEachLevelCell(const std::string& level, Fn&& fn)
	{
		int mapSize = 64;
		int x = 0, y = 0;

		for (size_t i = 0; i < level.size() && y < mapSize; )
		{
			char c = level[i];

			if (c == '\r') { ++i; continue; }
			if (c == '\n')
			{
				++y; x = 0; ++i;
				continue;
			}

			if (std::isdigit(static_cast<unsigned char>(c)))
			{
				//~ skips cells
				int skip = 0;
				while (i < level.size() && std::isdigit(static_cast<unsigned char>(level[i])))
				{
					skip = skip * 10 + (level[i] - '0');
					++i;
				}
				x += skip;
				if (x >= mapSize) { ++y; x = 0; }
				continue;
			}

			//~ occupied cells: 'w', 't', 's'
			if (c == 'w' || c == 't' || c == 's')
			{
				if (x < mapSize && y < mapSize)
					fn(x, y, c);
				++x;
				++i;
				if (x >= mapSize) { ++y; x = 0; }
				continue;
			}

			//~ any other char: empty cell
			++x; ++i;
			if (x >= mapSize) { ++y; x = 0; }
		}
	}

} // namespace pixel_game
