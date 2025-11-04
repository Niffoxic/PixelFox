#pragma once
#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_frame.h"
#include "pixel_engine/core/types.h"

#include "fox_math/matrix.h"
#include "fox_math/transform.h"
#include "fox_math/vector.h"

#include <string>
#include <cstdint>
#include <algorithm> 

namespace pixel_engine
{
    class PFE_API Camera2D final : public IFrameObject
    {
    public:
         Camera2D() noexcept;
        ~Camera2D() override = default;

        Camera2D(_In_ const Camera2D&) = delete;
        Camera2D(_Inout_ Camera2D&&)   = delete;

        Camera2D& operator=(_In_ const Camera2D&) = delete;
        Camera2D& operator=(_Inout_ Camera2D&&)   = delete;

        //~ IFrameObject interface implementation
        _NODISCARD std::string GetObjectName() const override;
        _NODISCARD bool        Initialize   ()       override;
        _NODISCARD bool        Release      ()       override;
        
        void OnFrameBegin(float deltaTime) override;
        void OnFrameEnd  ()                override;

        //~ Properties setters
        void SetPosition (_In_ const FVector2D& position)     noexcept;
        void AddPosition (_In_ const FVector2D& delta)        noexcept;
        void SetRotation (_In_ float radians)                 noexcept;
        void SetScale    (_In_ const FVector2D& scale)        noexcept;
        void SetTransform(_In_ const FTransform2D& transform) noexcept;

        //~ Properties getters
        _NODISCARD const FVector2D&    GetPosition () const noexcept;
        _NODISCARD float               GetRotation () const noexcept;
        _NODISCARD const FVector2D&    GetScale    () const noexcept;
        _NODISCARD const FTransform2D& GetTransform() const noexcept;

        _NODISCARD FVector2D WorldToCamera(
            _In_ const FVector2D& pWorld,
            _In_ const uint32_t& tile) const noexcept;
        
    private:
        //~ State
        FTransform2D m_transformCamera{};
    };
} // pixel_engine
