#pragma once

#include "player/player.h"
#include "core/vector.h"
#include "world/enemy_spawner/enemy_spawner.h"

#include <functional>
#include <sal.h>
#include <cfloat>

namespace pixel_game
{
	enum class EMapType
	{
		Finite,
		Infinite
	};

	typedef struct _MAP_BOUNDS
	{
		FVector2D min;
		FVector2D max;
	} MAP_BOUNDS;

	typedef struct _LOAD_SCREEN_DETAILS
	{
		_In_ pixel_engine::PEFont* pLoadTitle;
		_In_ pixel_engine::PEFont* pLoadDescription;
	} LOAD_SCREEN_DETAILS;

	typedef struct _MAP_INIT_DESC
	{
		_In_ pixel_engine::PEKeyboardInputs* pKeyboard;
		_In_ LOAD_SCREEN_DETAILS   LoadScreen;
		_In_ PlayerCharacter*	   pPlayerCharacter;
		_In_ EnemySpawner*		   pEnemySpawner;
		_In_ float				   MapDuration;
		_In_ bool				   UseBounds;
		_In_ MAP_BOUNDS			   Bounds;
		_In_ EMapType			   Type;
		_In_ std::function<void()> OnMapComplete;
	} MAP_INIT_DESC;

	class IMap
	{
	public:
		virtual ~IMap() = default;

		virtual void Initialize(_In_ const MAP_INIT_DESC& desc) = 0;
		virtual void Update(_In_ float deltaTime) = 0;
		virtual void Release() = 0;

		virtual void Start  () = 0;
		virtual void Pause  () = 0;
		virtual void Resume () = 0;
		virtual void Restart() = 0;
		virtual void UnLoad () = 0;

		_NODISCARD virtual bool		  IsInitialized() const = 0;
		_NODISCARD virtual bool		  IsActive	   () const = 0;
		_NODISCARD virtual bool		  IsPaused     () const = 0;
		_NODISCARD virtual bool		  IsComplete   () const = 0;
		_NODISCARD virtual float	  GetElapsed   () const = 0;
		_NODISCARD virtual float	  GetRemaining () const = 0;
		_NODISCARD virtual bool		  HasBounds	   () const = 0;
		_NODISCARD virtual MAP_BOUNDS GetBounds	   () const = 0;
		_NODISCARD virtual EMapType   GetType	   () const = 0;
	};
}
