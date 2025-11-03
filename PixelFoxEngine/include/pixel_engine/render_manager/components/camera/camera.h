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
        void SetRotation (_In_ float radians)                 noexcept;
        void SetZoom     (_In_ float zoom)                    noexcept;
        void SetScale    (_In_ const FVector2D& scale)        noexcept;
        void SetTransform(_In_ const FTransform2D& transform) noexcept;

        //~ Properties getters
        _NODISCARD const FVector2D&    GetPosition () const noexcept;
        _NODISCARD float               GetRotation () const noexcept;
        _NODISCARD float               GetZoom     () const noexcept;
        _NODISCARD const FVector2D&    GetScale    () const noexcept;
        _NODISCARD const FTransform2D& GetTransform() const noexcept;

        //~ Viewport in pixel space setters
        void SetViewportSize  (_In_ uint32_t width, _In_ uint32_t height) noexcept;
        void SetViewportOrigin(_In_ const FVector2D& originPx)            noexcept;
        
        //~ Viewport in pixel sapce getters
        _NODISCARD uint32_t         GetViewportWidth () const noexcept;
        _NODISCARD uint32_t         GetViewportHeight() const noexcept;
        _NODISCARD const FVector2D& GetViewportOrigin() const noexcept;

        //~ iff y screen down
                   void SetScreenYDown(_In_ bool yDown) noexcept;
        _NODISCARD bool IsScreenYDown ()          const noexcept;

        //~ Spaces and Matrices
        _NODISCARD FMatrix2DAffine GetViewMatrix   () const noexcept;
        _NODISCARD FMatrix2DAffine GetInvViewMatrix() const noexcept;
        _NODISCARD FMatrix2DAffine GetScreenMatrix () const noexcept;

        //~ World to and from Screen mapping
        _NODISCARD FVector2D WorldToView  (_In_ const FVector2D& pWorld)  const noexcept;
        _NODISCARD FVector2D ViewToWorld  (_In_ const FVector2D& pView)   const noexcept;
        _NODISCARD FVector2D WorldToScreen(_In_ const FVector2D& pWorld)  const noexcept;
        _NODISCARD FVector2D WorldToScreen(
            _In_ const FVector2D& pWorld,
            _In_ const uint32_t& tile) const noexcept;
        _NODISCARD FVector2D ScreenToWorld(_In_ const FVector2D& pScreen) const noexcept;

        //~ for culling
        _NODISCARD PFE_RECT ScreenRectToWorldAABB() const noexcept;

        //~ features TODO: make it dynamic instead of hard coded
        void Pan               (_In_ const FVector2D& deltaWorld)                   noexcept;
        void ZoomAtScreenPoint (_In_ float factor, _In_ const FVector2D& screenPt)  noexcept;
        void RotateAtWorldPoint(_In_ float deltaRad, _In_ const FVector2D& worldPt) noexcept;

        void SetFollowTarget   (_In_opt_ const FVector2D* target) noexcept;
        void SetFollowSmoothing(_In_ float smooth01)              noexcept;
        void SetWorldBounds    (_In_ const PFE_RECT& bounds)      noexcept;
        void EnableWorldClamp  (_In_ bool enable)                 noexcept;

        _NODISCARD PFE_RECT GetWorldBounds     () const noexcept;
        _NODISCARD float    GetFollowSmoothing () const noexcept;
        _NODISCARD bool     IsWorldClampEnabled() const noexcept;

        //~ shaking effect
        void StartShake(
            _In_ float amplitudePx,
            _In_ float frequencyHz,
            _In_ float durationSec) noexcept;

                   void StopShake()       noexcept;
        _NODISCARD bool IsShaking() const noexcept;

    private:
        void RebuildCached() noexcept;
        void ApplyClamp   () noexcept;
        
        //~ Helpers
        void FollowTarget     (float delatTime) noexcept;
        void ShakeCameraEffect(float deltaTime) noexcept;
        
        std::pair<float, float> GetShakeNoise() const noexcept;

    private:
        //~ State
        FTransform2D m_transformCamera{};
        FVector2D    m_vecViewOriginPx{ 0.f, 0.f };
        uint32_t     m_nViewWidth     { 0 };
        uint32_t     m_nViewHeight    { 0 };
        bool         m_bYDown         { true };
        bool         m_bDirty         { true };

        //~ Cached
        float           m_nZoom{ 1.0f };
        FMatrix2DAffine m_matView;
        FMatrix2DAffine m_matInvView;
        FMatrix2DAffine m_matScreen;

        //~ Camera Effects for player TODO: Make it dynamic
        //~ follow
        const FVector2D* m_pFollowTarget  { nullptr };
        float            m_nFollowSmooth  { 0.15f };
        PFE_RECT         m_rectWorldBounds{ 0, 0, 0, 0 };
        bool             m_bWorldClamp    { false };

        //~ shake
        float m_nShakeAmp      { 0.f };
        float m_nShakeFreq     { 0.f };
        float m_nShakeTime     { 0.f };
        float m_nShakeTotalTime{ 0.f };
    };
} // pixel_engine
