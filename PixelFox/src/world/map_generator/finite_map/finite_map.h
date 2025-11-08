#pragma once

#include "world/map_generator/interface_map.h"
#include "obsticle/obsiticle.h"

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
		void SetPlayerSpawnPosition();
		
		//~ build obsticles
		void BuildTrees     (_In_ LOAD_SCREEN_DETAILS details);
		void BuildStones	(_In_ LOAD_SCREEN_DETAILS details);

		//~ generate water
		void BuildWaters	(_In_ LOAD_SCREEN_DETAILS details);
		bool InBounds(_In_ const FVector2D& p) const;
		bool TrySpawnWaterTile(_In_ const FileData& data, _In_ const FVector2D& pos);

		fox::vector<FVector2D> MakeWaterLine(_In_ int n) const;
		fox::vector<FVector2D> MakeWaterBlock(_In_ int w, _In_ int h) const;
		fox::vector<FVector2D> MakeWaterBlob(_In_ int n);

		void BuildWaterCluster(
			_In_ const FileData& data,
			_In_ const FVector2D& start,
			_In_ const fox::vector<FVector2D>& offsets,
			_Inout_ int& totalSpawned,
			_In_ int spawnLimit,
			_Inout_ LOAD_SCREEN_DETAILS& details);


		//~ GUI
		void BuildMapGUI(LOAD_SCREEN_DETAILS details);

		FVector2D GetRandomPositionInBound();
		int GetRandomNumber(int min, int max);

	private:
		fox::vector<std::unique_ptr<Obsticle>> m_ppObsticle{};

		PlayerCharacter* m_pPlayerCharacter{ nullptr };
		EnemySpawner*	 m_pEnemySpawner{ nullptr };

		bool   m_bInitialized{ false };
		bool   m_bActive  { false };
		bool   m_bPaused  { false };
		bool   m_bComplete{ false };

		bool   m_bUseBounds  { false };
		float  m_nMapDuration{ 0.0f };
		float  m_nElapsedTime{ 0.0f };

		EMapType   m_eType { EMapType::Finite };
		MAP_BOUNDS m_Bounds{};

		std::function<void()> m_OnMapComplete{};

		//~ GUI

		//~ hard coded file location
		fox::vector<FileData> m_ppszTress {};
		fox::vector<FileData> m_ppszWater {};
		fox::vector<FileData> m_ppszStones{};
	};
} // namespace pixel_game
