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
#include "pixel_engine/core/interface/interface_frame.h"

#include "core/vector.h"

namespace pixel_engine
{
	class PFE_API TileAnim final : public IFrameObject
	{
	public:
		explicit TileAnim(_Inout_ PEISprite* body);
		~TileAnim() override = default;

		TileAnim(const TileAnim&) = default;
		TileAnim(TileAnim&&) = default;

		TileAnim& operator=(const TileAnim&) = default;
		TileAnim& operator=(TileAnim&&) = default;

		//~ Frame Object Interface Impl
		_NODISCARD _Check_return_
		std::string GetObjectName() const override { return "TileAnim"; }

		_NODISCARD _Check_return_
		bool Initialize() override { return true; }
		bool Release   () override { return true; }

		void OnFrameBegin(_In_ float deltaTime) override;
		void OnFrameEnd  () override;

		void Build(); // after configuring TileAnim
		void EnableLoop(bool loop);
		void SetCycleCompleteDuration(float seconds);
		bool IsCycleComplete() const;
		void Play();
		void Stop();

		void AddFrame(const std::string& path, float startTime = 0.0f);

		bool IsBuilt() const;

	private:
		void PrepareAnimation();

	private:
		bool	   m_bBuilt{ false };
		PEISprite* m_pSpriteBody{ nullptr };

		struct FrameInformation
		{
			std::string filepath;
			float		frameStartTime;
		};
		fox::vector<FrameInformation> m_ppFrameInformations{};

		struct FrameData
		{
			FrameInformation info;
			Texture* sampledTexture;
		};
		fox::vector<FrameData> m_ppFrames{};

		struct AnimState
		{
			float cycleDuration{ 1.0f };
			float TotalTime{ 0.0f };
			int   currentFrame{ 0 };
			bool  playing{ true };
			bool  shouldLoop{ true };
			bool  cycleCompleted{ false };
		} m_AnimState;
	};
} // namespace pixel_engine
