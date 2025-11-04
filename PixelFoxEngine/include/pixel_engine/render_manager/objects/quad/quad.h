#pragma once
#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_sprite.h"


namespace pixel_engine
{
    class PFE_API QuadObject final : public PEISprite
    {
    public:
        QuadObject() = default;
        ~QuadObject() override = default;

        _NODISCARD _Check_return_
        std::string GetObjectName() const override;

        _NODISCARD _Check_return_
        bool Initialize() override;
        bool Release() override;

        void Update(float deltaTime, const PFE_WORLD_SPACE_DESC& space) override;

        void SetTransform(_In_ const FTransform2D& t) override;
        _NODISCARD _Check_return_
        const FTransform2D& GetTransform() const override;

        _NODISCARD _Check_return_
        FMatrix2DAffine GetAffineMatrix() const override;

        void SetPosition(float x, float y)        override;
        void SetRotation(float radians)           override;
        void SetScale   (float sx, float sy)      override;
        void SetPivot   (float px, float py)      override;
        void SetTexture (const std::string& path) override;

        _NODISCARD _Check_return_ fox_math::Vector2D<float> GetPosition() const override;
        _NODISCARD _Check_return_ float                     GetRotation() const override;
        _NODISCARD _Check_return_ fox_math::Vector2D<float> GetScale()    const override;
        _NODISCARD _Check_return_ fox_math::Vector2D<float> GetPivot()    const override;

        void SetVisible(bool v)   override;
        _NODISCARD _Check_return_ bool     IsVisible() const override;

        void SetLayer(uint32_t l) override;
        _NODISCARD _Check_return_ uint32_t GetLayer() const override;

        _NODISCARD _Check_return_
        Texture* GetTexture() const override;

        //~ relative to the camera
        _NODISCARD _Check_return_
        FVector2D GetUAxisRelativeToCamera() const noexcept override;
        _NODISCARD _Check_return_
        FVector2D GetVAxisRelativeToCamera() const noexcept override;
        _NODISCARD _Check_return_
        FVector2D GetPositionRelativeToCamera() const noexcept override;
    private:
        void UpdateObjectToCameraSpace(const PFE_WORLD_SPACE_DESC& space);

    private:
        std::string m_szTexturePath{};
        Texture*    m_pTexture     { nullptr };
        
        //~ Test only
        std::unique_ptr<Texture> m_pSampledTexture{ nullptr };

        FTransform2D              m_transform{};
        bool                      m_visible{ true };
        uint32_t                  m_layer{ 0 };

        //~ relative to the camera
        FVector2D m_ObjectCameraAxisU       {};
        FVector2D m_ObjectCameraAxisV       {};
        FVector2D m_ObjectCameraViewPosition{};
    };
} // namespace pixel_engine
