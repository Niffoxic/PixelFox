// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxEngineAPI.h"
#include "pixel_engine/utilities/id_allocator.h"

#include "fox_math/transform.h"
#include "fox_math/matrix.h"

#include <string>
#include <sal.h>

#include "pixel_engine/render_manager/components/camera/camera.h"
#include "pixel_engine/render_manager/components/texture/resource/texture.h"

namespace pixel_engine
{
	enum class PFE_API ELayer
	{
		Background	= 0,
		Obstacles	= 1,
		Npc_Deco	= 2,
		Npc_AI		= 3,
		Player		= 4,
		Font        = 5
	};

	typedef struct _PFE_WORLD_SPACE_DESC
	{
		_Inout_ Camera2D* pCamera;
		_In_ FVector2D Origin;
		_In_ FVector2D X1;
		_In_ FVector2D Y1;
	} PFE_WORLD_SPACE_DESC;

	/// <summary>
	/// Object interface for tile based entities
	/// </summary>
	class PFE_API PEISprite
	{
	public:
		PEISprite() : m_idAllocated(IDAllocator<PEISprite>::AllocateID())
		{}

		virtual ~PEISprite() = default;

		//~ ID Accessors
		_NODISCARD _Check_return_
		UniqueId GetInstanceID() const noexcept { return m_idAllocated.ID; }

		_NODISCARD _Check_return_
		UniqueId GetTypeId() const noexcept { return m_idAllocated.Type; }

		_NODISCARD _Check_return_
		virtual std::string GetObjectName() const = 0;

		_NODISCARD _Check_return_
		virtual bool Initialize() = 0; //~ called once after initalizing
		virtual bool Release()    = 0; // release any upholding resources

		virtual void Update(
			_In_ float deltaTIme,
			_In_ const PFE_WORLD_SPACE_DESC& space) = 0; // called every frame

		virtual void SetTransform(_In_ const FTransform2D& t) = 0;
		
		_NODISCARD _Check_return_
		virtual const FTransform2D& GetTransform() const = 0;

		_NODISCARD _Check_return_
		virtual FMatrix2DAffine GetAffineMatrix () const = 0;

		//~ transform
		virtual void SetPosition(_In_ float x, _In_ float y)   = 0;
		virtual void SetRotation(_In_ float radians)		   = 0;
		virtual void SetScale   (_In_ float sx, _In_ float sy) = 0;
		virtual void SetPivot   (_In_ float px, _In_ float py) = 0;
		virtual void SetTexture (_In_ const std::string& path) = 0;
		virtual void SetTexture (_Inout_ Texture* rawTexture)  = 0;

		_NODISCARD _Check_return_
		virtual FVector2D GetPosition() const	 = 0;
		_NODISCARD _Check_return_
		virtual float    GetRotation () const	 = 0;
		_NODISCARD _Check_return_
		virtual FVector2D GetScale	 () const	 = 0;
		_NODISCARD _Check_return_
		virtual FVector2D GetPivot   () const	 = 0;

		virtual void SetVisible(_In_ bool v)	 = 0;
		_NODISCARD _Check_return_
		virtual bool IsVisible () const			 = 0;

		virtual void SetLayer(_In_ ELayer layer) = 0;
		_NODISCARD _Check_return_
		virtual ELayer GetLayer() const			 = 0;

		void MarkDirty(_In_ bool flag) const noexcept { m_bDirty = flag; }
		
		_NODISCARD _Check_return_ 
		bool IsDirty() const noexcept { return m_bDirty; }

		_NODISCARD _Check_return_
		virtual Texture* GetTexture() const = 0;

		//~ relative to the camera
		_NODISCARD _Check_return_
		virtual FVector2D GetUAxisRelativeToCamera   () const noexcept = 0;
		_NODISCARD _Check_return_
		virtual FVector2D GetVAxisRelativeToCamera   () const noexcept = 0;
		_NODISCARD _Check_return_
		virtual FVector2D GetPositionRelativeToCamera() const noexcept = 0;
	
		_NODISCARD _Check_return_ __forceinline
		bool NeedSampling() const { return m_bResampleNeeded; }

		void AssignSampledTexture(_Inout_ Texture* texture)
		{
			m_pSampledTexture = texture;
			m_bResampleNeeded = false;
		}

		_NODISCARD _Check_return_
		Texture* GetSampledTexture() const { return m_pSampledTexture; }

	protected:
		bool	 m_bResampleNeeded{ true };

	private:
		Texture* m_pSampledTexture{ nullptr };

		mutable bool m_bDirty{ true };
		AllocatedID  m_idAllocated;
	};
} // namespace pixel_engine
