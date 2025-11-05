// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "anim.h"

#include "pixel_engine/render_manager/components/texture/allocator/texture_resource.h"
#include "pixel_engine/render_manager/render_queue/sampler/sample_allocator.h"

#include <ranges>
#include <cmath>

pixel_engine::TileAnim::TileAnim(pixel_engine::PEISprite* body)
	: m_pSpriteBody(body)
{
}

void pixel_engine::TileAnim::OnFrameBegin(float deltaTime)
{
	if (not IsBuilt()) Build();
	if (not m_AnimState.playing) return;

	m_AnimState.TotalTime += deltaTime;

	if (not m_AnimState.shouldLoop &&
		m_AnimState.TotalTime >= m_AnimState.cycleDuration)
	{
		m_AnimState.currentFrame = m_ppFrames.size() - 1;
		m_AnimState.cycleCompleted = true;
		m_AnimState.playing = false;

		m_pSpriteBody->SetTexture(m_ppFrames.back().sampledTexture);
	}

	if (m_AnimState.shouldLoop &&
		m_AnimState.TotalTime >= m_AnimState.cycleDuration)
	{
		m_AnimState.TotalTime = fmod(m_AnimState.TotalTime,
			m_AnimState.cycleDuration);
		m_AnimState.cycleCompleted = false;
	}

	//~ extract current frame
	float frameTime = 0.0f;
	for (int i = 0; i < m_ppFrames.size(); i++)
	{
		frameTime += m_ppFrames[i].info.frameStartTime;

		if (m_AnimState.TotalTime < frameTime ||
			i == m_ppFrames.size() - 1)
		{
			if (m_AnimState.currentFrame != i)
			{
				m_AnimState.currentFrame = i;
				if (m_pSpriteBody)
				{
					m_pSpriteBody->SetTexture(m_ppFrames[i].sampledTexture);
				}
			}
			return; // update already so leaving
		}
	}
}

void pixel_engine::TileAnim::OnFrameEnd()
{
}

void pixel_engine::TileAnim::Build()
{
	if (m_bBuilt) return;
	m_ppFrames.clear();

	for (auto& info : m_ppFrameInformations)
	{
		Texture* texture = TextureResource::Instance().LoadTexture(info.filepath);
		if (!texture || !m_pSpriteBody) continue;

		PFE_CREATE_SAMPLE_TEXTURE sampleDesc{};
		sampleDesc.scaledBy = m_pSpriteBody->GetScale();
		sampleDesc.texture = texture;
		sampleDesc.tileSize = 32;
		Texture* sampledTexture = Sampler::Instance().BuildTexture(sampleDesc);

		FrameData data{};
		data.info = info;
		data.sampledTexture = sampledTexture;
		m_ppFrames.push_back(data);
	}

	PrepareAnimation();
	m_bBuilt = true;
}

void pixel_engine::TileAnim::EnableLoop(bool loop)
{
	m_AnimState.shouldLoop = loop;
}

void pixel_engine::TileAnim::SetCycleCompleteDuration(float seconds)
{
	m_AnimState.cycleDuration = seconds;
}

bool pixel_engine::TileAnim::IsCycleComplete() const
{
	return m_AnimState.cycleCompleted;
}

void pixel_engine::TileAnim::Play()
{
	m_AnimState.playing = true;
	m_AnimState.cycleCompleted = false;
	m_AnimState.currentFrame = 0.0f;
	m_AnimState.TotalTime = 0.0f;
}

void pixel_engine::TileAnim::Stop()
{
	m_AnimState.playing = false;
	m_AnimState.cycleCompleted = false;
	m_AnimState.currentFrame = 0.0f;
	m_AnimState.TotalTime = 0.0f;
}

void pixel_engine::TileAnim::AddFrame(const std::string& path, float startTime)
{
	FrameInformation info{};
	info.filepath = path;
	info.frameStartTime = startTime;

	m_ppFrameInformations.push_back(info);
}

bool pixel_engine::TileAnim::IsBuilt() const
{
	return m_bBuilt;
}

void pixel_engine::TileAnim::PrepareAnimation()
{
	if (m_ppFrames.empty()) return;

	float deltaTime = m_AnimState.cycleDuration /
		static_cast<float>(m_ppFrames.size());

	for (auto& frame : m_ppFrames)
	{
		frame.info.frameStartTime = deltaTime;
	}
}