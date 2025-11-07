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

#include "pixel_engine/core/interface/interface_sprite.h"

namespace pixel_engine
{
    class PFE_API QuadObject final : public PEISprite
    {
    public:
         QuadObject()          = default;
        ~QuadObject() override = default;

        _NODISCARD _Check_return_
        std::string GetObjectName() const override;

        _NODISCARD _Check_return_
        bool Initialize() override;
        bool Release   () override;

        void Update(
            _In_ float deltaTime,
            _In_ const PFE_WORLD_SPACE_DESC& space)   override;
        
        void SetTransform(_In_ const FTransform2D& t) override;
        
        _NODISCARD _Check_return_
        const FTransform2D& GetTransform() const override;

        _NODISCARD _Check_return_
        FMatrix2DAffine GetAffineMatrix () const override;

        void SetPosition(_In_ float x, _In_ float y)     override;
        void SetPosition(_In_ const FVector2D& position) override;
        void SetRotation(_In_ float radians)             override;
        void SetScale   (_In_ float sx, _In_ float sy)   override;
        void SetScale   (_In_ const FVector2D& scale)    override;
        void SetPivot   (_In_ float px, _In_ float py)   override;
        void SetTexture (_In_ const std::string& path)   override;
        void SetTexture (_Inout_ Texture* rawTexture)    override;

        _NODISCARD _Check_return_ FVector2D GetPosition() const override;
        _NODISCARD _Check_return_ float     GetRotation() const override;
        _NODISCARD _Check_return_ FVector2D GetScale   () const override;
        _NODISCARD _Check_return_ FVector2D GetPivot   () const override;

        void SetVisible(_In_ bool v) override;
        
        _NODISCARD _Check_return_
        bool IsVisible()       const override;

        void SetLayer(_In_ ELayer l) override;
        
        _NODISCARD _Check_return_ 
        ELayer GetLayer    () const override;

        _NODISCARD _Check_return_
        Texture* GetTexture() const override;

        //~ relative to the camera
        _NODISCARD _Check_return_
        FVector2D GetUAxisRelativeToCamera   () const noexcept override;
        
        _NODISCARD _Check_return_
        FVector2D GetVAxisRelativeToCamera   () const noexcept override;
        
        _NODISCARD _Check_return_
        FVector2D GetPositionRelativeToCamera() const noexcept override;
    
    private:
        void UpdateObjectToCameraSpace(_In_ const PFE_WORLD_SPACE_DESC& space);

    private:
        std::string  m_szTexturePath{};
        Texture*     m_pTexture     { nullptr };
        bool         m_visible      { true };
        ELayer       m_layer        { ELayer::Obstacles };
        FVector2D    m_scale{ 1.f, 1.f };

        //~ relative to the camera
        FVector2D m_ObjectCameraAxisU       {};
        FVector2D m_ObjectCameraAxisV       {};
        FVector2D m_ObjectCameraViewPosition{};
    };
} // namespace pixel_engine
