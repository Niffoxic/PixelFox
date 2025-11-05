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
#include "anim_state.h"
#include "core/vector.h"

#include <filesystem>
#include <string>
#include <algorithm>

using namespace pixel_engine;

AnimSateMachine::AnimSateMachine(PEISprite* sprite)
	: m_pSprite(sprite)
{
}

_Use_decl_annotations_
bool pixel_engine::AnimSateMachine::Initialize()
{
	for (auto state : m_states)
	{
		if (state.second) state.second->Build();
	}
	return true;
}

_Use_decl_annotations_
void pixel_engine::AnimSateMachine::OnFrameBegin(float deltaTime)
{
	if (m_states.contains(m_szCurrentState) &&
		m_states[m_szCurrentState] != nullptr)
	{
		m_states[m_szCurrentState]->OnFrameBegin(deltaTime);
	}
}

void pixel_engine::AnimSateMachine::OnFrameEnd()
{
}

void pixel_engine::AnimSateMachine::AddState(const std::string& stateName)
{
	if (m_states.contains(stateName)) return;
	if (!m_pSprite) return;

	m_states[stateName] = std::make_unique<TileAnim>(m_pSprite);
}

void pixel_engine::AnimSateMachine::AddState(const std::string& stateName, std::unique_ptr<TileAnim> anim)
{
	if (!anim) return;
	m_states[stateName] = std::move(anim);
}

void pixel_engine::AnimSateMachine::AddFrame(const std::string& stateName, const std::string& path)
{
	if (!m_states.contains(stateName)) return;
	m_states[stateName]->AddFrame(path);
}

void pixel_engine::AnimSateMachine::AddFrameFromDir(
	const std::string& stateName,
	const std::string& dirPath
) 
{
	if (!m_states.contains(stateName)) AddState(stateName);

	std::vector<std::filesystem::path> files;
	for (const auto& entry : std::filesystem::directory_iterator(dirPath))
	{
		const auto& p = entry.path();
		if (p.extension() == ".png") files.push_back(p);
	}

	std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b)
	{
		auto get_index = [](const std::filesystem::path& p)
		{
			const auto name = p.stem().string();
			const auto pos = name.find_last_of('_');
			return (pos != std::string::npos) ? std::stoi(name.substr(pos + 1)) : 0;
		};
		return get_index(a) < get_index(b);
	});

	for (const auto& file : files) AddFrame(stateName, file.string());
}

void pixel_engine::AnimSateMachine::SetInitialState(const std::string& name)
{
	if (!m_states.contains(name)) return;
	
	m_szCurrentState = name;
	m_states[name]->Play();
}

void pixel_engine::AnimSateMachine::TransitionTo(const std::string& name)
{
	if (name == m_szCurrentState || !m_states.contains(name))
		return;

	if (m_fnOnExitCallbacks.contains(m_szCurrentState))
	{
		if (m_fnOnExitCallbacks[m_szCurrentState])
		{
			m_fnOnExitCallbacks[m_szCurrentState]();
		}
	}

	if (!m_states[m_szCurrentState]) return;
	if (!m_states[m_szCurrentState]->IsBuilt()) return;

	m_states[m_szCurrentState]->Stop();
	m_szPreviousState = m_szCurrentState;

	m_szCurrentState = name;
	m_states[name]->Play();

	if (m_fnOnEnterCallbacks.contains(name))
	{
		if (m_fnOnEnterCallbacks[name])
		{
			m_fnOnEnterCallbacks[name]();
		}
	}
}

TileAnim* pixel_engine::AnimSateMachine::GetTileAnim(const std::string& name)
{
	if (m_states.contains(name)) return m_states[name].get();
	return nullptr;
}

const std::string& pixel_engine::AnimSateMachine::GetCurrentState() const
{
	return m_szCurrentState;
}

const std::string& pixel_engine::AnimSateMachine::GetPreviousState() const
{
	return m_szPreviousState;
}

bool pixel_engine::AnimSateMachine::IsInState(const std::string& name) const
{
	return name == m_szCurrentState;
}

void pixel_engine::AnimSateMachine::SetOnEnterCallback(const std::string& state, std::function<void()> callback)
{
	m_fnOnEnterCallbacks[state] = std::move(callback);
}

void pixel_engine::AnimSateMachine::SetOnExitCallback(const std::string& state, std::function<void()> callback)
{
	m_fnOnExitCallbacks[state] = std::move(callback);
}
