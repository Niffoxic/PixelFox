#pragma once
#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_object.h"


namespace pixel_engine
{
    class PFE_API QuadObject final : public PEIObject
    {
    public:
        QuadObject() = default;
        ~QuadObject() override = default;

        _NODISCARD _Check_return_
        std::string GetObjectName() const override;

        _NODISCARD _Check_return_
        bool Initialize() override;
        bool Release() override;

        void Update(float deltaTime) override {}

        void SetTransform(_In_ const FTransform2D& t) override;
        _NODISCARD _Check_return_
        const FTransform2D& GetTransform() const override;

        _NODISCARD _Check_return_
        FMatrix2DAffine GetAffineMatrix() const override;

        void SetPreAffine(_In_ const FMatrix2DAffine& m);
        void ClearPreAffine();
        _NODISCARD _Check_return_ bool HasPreAffine() const noexcept;

        void SetPostAffine(_In_ const FMatrix2DAffine& m);
        void ClearPostAffine();
        _NODISCARD _Check_return_ bool HasPostAffine() const noexcept;

        void SetPosition(float x, float y) override;
        void SetRotation(float radians)    override;
        void SetScale(float sx, float sy)  override;
        void SetPivot(float px, float py)  override;

        _NODISCARD _Check_return_ fox_math::Vector2D<float> GetPosition() const override;
        _NODISCARD _Check_return_ float                     GetRotation() const override;
        _NODISCARD _Check_return_ fox_math::Vector2D<float> GetScale()    const override;
        _NODISCARD _Check_return_ fox_math::Vector2D<float> GetPivot()    const override;

        void SetUnitSize(float widthUnits, float heightUnits) override;
        _NODISCARD _Check_return_
        fox_math::Vector2D<float> GetUnitSize() const override;

        void SetVisible(bool v)   override;
        _NODISCARD _Check_return_ bool     IsVisible() const override;

        void SetLayer(uint32_t l) override;
        _NODISCARD _Check_return_ uint32_t GetLayer() const override;

    private:
        FTransform2D              m_base{};

        bool                      m_hasPre{ false };
        bool                      m_hasPost{ false };
        FMatrix2DAffine           m_pre{};
        FMatrix2DAffine           m_post{};

        fox_math::Vector2D<float> m_unitSize{ 1.0f, 1.0f };

        bool                      m_visible{ true };
        uint32_t                  m_layer{ 0 };
    };
} // namespace pixel_engine
