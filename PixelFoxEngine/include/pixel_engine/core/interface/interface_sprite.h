#pragma once

#include "PixelFoxEngineAPI.h"
#include "pixel_engine/utilities/id_allocator.h"

#include "fox_math/transform.h"
#include "fox_math/matrix.h"

#include <string>
#include <sal.h>

namespace pixel_engine
{
	/// <summary>
	/// Object interface for tile based entities
	/// </summary>
	class PFE_API PEISprte
	{
	public:
		PEISprte() : m_idAllocated(IDAllocator<PEISprte>::AllocateID())
		{
		}

		virtual ~PEISprte() = default;

		//~ ID Accessors
		_NODISCARD _Check_return_
			UniqueId GetInstanceID() const noexcept { return m_idAllocated.ID; }

		_NODISCARD _Check_return_
			UniqueId GetTypeId() const noexcept { return m_idAllocated.Type; }

		_NODISCARD _Check_return_
			virtual std::string GetObjectName() const = 0;

		_NODISCARD _Check_return_
			virtual bool Initialize() = 0; //~ called once after initalizing
		virtual bool Release() = 0; // release any upholding resources

		virtual void Update(float deltaTIme) = 0; // called every frame

		virtual void SetTransform(_In_ const FTransform2D& t) = 0;
		_NODISCARD _Check_return_
			virtual const FTransform2D& GetTransform() const = 0;

		_NODISCARD _Check_return_
			virtual FMatrix2DAffine GetAffineMatrix() const = 0;

		//~ transform
		virtual void SetPosition(float x, float y) = 0;
		virtual void SetRotation(float radians) = 0;  // Z-rotation
		virtual void SetScale(float sx, float sy) = 0;
		virtual void SetPivot(float px, float py) = 0;

		_NODISCARD _Check_return_
			virtual fox_math::Vector2D<float> GetPosition() const = 0;
		_NODISCARD _Check_return_
			virtual float                     GetRotation() const = 0;
		_NODISCARD _Check_return_
			virtual fox_math::Vector2D<float> GetScale()    const = 0;
		_NODISCARD _Check_return_
			virtual fox_math::Vector2D<float> GetPivot()    const = 0;

		virtual void SetUnitSize(float widthUnits, float heightUnits) = 0;
		_NODISCARD _Check_return_
			virtual fox_math::Vector2D<float> GetUnitSize() const = 0;

		virtual void SetVisible(bool v) = 0;
		_NODISCARD _Check_return_
			virtual bool IsVisible() const = 0;

		virtual void SetLayer(uint32_t l) = 0;
		_NODISCARD _Check_return_
			virtual uint32_t GetLayer() const = 0;

	private:
		AllocatedID m_idAllocated;
	};
} // namespace pixel_engine
