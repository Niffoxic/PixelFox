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
#include "pixel_engine/core/interface/interface_frame.h"
#include "pixel_engine/core/interface/interface_sprite.h"

#include "core/unordered_map.h"

#include "pixel_engine/render_manager/components/animator/anim.h"

namespace pixel_engine 
{
	class PFE_API AnimSateMachine final: public IFrameObject
	{
	public:
        explicit AnimSateMachine(PEISprite* sprite);
        ~AnimSateMachine() override = default;

        //~ interface implementation
        _NODISCARD _Check_return_
        std::string GetObjectName() const override { return "AnimStateMachine"; }

        _NODISCARD _Check_return_
        bool Initialize() override;
        bool Release   () override { return true; }

        void OnFrameBegin(_In_ float deltaTime) override;
        void OnFrameEnd  () override;

        void AddState(const std::string& stateName);        
        void AddState(
            const std::string& stateName,
            std::unique_ptr<TileAnim> anim);

        void AddFrame(
            const std::string& stateName,
            const std::string& path);

        /// <summary>
        /// Extracts all .png files from the dir for building frame
        /// Must be seprated by '_' like name_index.png
        /// </summary>
        void AddFrameFromDir(
            const std::string& stateName,
            const std::string& dirPath
        );

        void SetInitialState(const std::string& name);
        void TransitionTo   (const std::string& name);

        TileAnim* GetTileAnim(const std::string& name);
        const std::string& GetCurrentState () const;
        const std::string& GetPreviousState() const;
        bool IsInState(const std::string& name) const;

        void SetOnEnterCallback(
            const std::string& state,
            std::function<void()> callback);

        void SetOnExitCallback(
            const std::string& state,
            std::function<void()> callback);

    private:
        PEISprite* m_pSprite{ nullptr };

        fox::unordered_map<std::string, std::unique_ptr<TileAnim>> m_states;
        fox::unordered_map<std::string, std::function<void()>>      m_fnOnEnterCallbacks;
        fox::unordered_map<std::string, std::function<void()>>      m_fnOnExitCallbacks;

        std::string m_szCurrentState;
        std::string m_szPreviousState;
	};

} // namespace pixel_engine
