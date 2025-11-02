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

        _NODISCARD _Check_return_
        bool BuildDiscreteGrid(float step, PFE_SAMPLE_GRID_2D& gridOut) const override;

    private:
        void RebuildIfDirty(_In_opt_ const Camera2D* camera) const;
        void UpdateWorldPosition(const PFE_WORLD_SPACE_DESC& space) const;

    private:
        FTransform2D              m_base{};

        bool                      m_hasPre{ false };
        bool                      m_hasPost{ false };
        FMatrix2DAffine           m_pre{};
        FMatrix2DAffine           m_post{};

        mutable FMatrix2DAffine   m_cachedWorld{};
        fox_math::Vector2D<float> m_unitSize{ 1.0f, 1.0f };

        bool                      m_visible{ true };
        uint32_t                  m_layer{ 0 };

        //~ cache grid
        mutable FVector2D m_SuScreen{};
        mutable FVector2D m_SvScreen{};
        mutable FVector2D m_baseScreen{};
        mutable bool      m_bScreenDirty{ true };
        mutable Camera2D*         m_pLastCamera{ nullptr };
    };
} // namespace pixel_engine
