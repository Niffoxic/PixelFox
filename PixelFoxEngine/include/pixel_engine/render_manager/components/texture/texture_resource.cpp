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
#include "texture_resource.h"
    
#include "png_loader/png_loader.h"
#include "pixel_engine/exceptions/base_exception.h"

#include <fstream>

_Use_decl_annotations_
pixel_engine::Texture* pixel_engine::TextureResource::LoadTexture(
    _In_ const std::string& path)
{
    //~ Check if file exists
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        std::string message = "Texture file not found: " + path;
        THROW_MSG(message.c_str());
    }

    if (m_cacheTextures.contains(path))
        return m_cacheTextures[path].get();
    
    const size_t dotPos = path.find_last_of('.');
    std::string  extension;
    
    //~ Find Extension
    if (dotPos != std::string::npos)
    {
        extension = path.substr(dotPos + 1);
        for (auto& c : extension) c = static_cast<char>(std::tolower(c));
    }

    if (extension == "png")
    {
        auto texture = std::move(PNGLoader::Instance().LoadTexture(path));
        if (!texture)
            THROW_MSG(("Failed to load PNG texture: " + path).c_str());

        Texture* raw = texture.get();
        m_cacheTextures[path] = std::move(texture);
        return raw;
    }

    std::string message = "The extension ." + extension + " is not implemented yet!";
    THROW_MSG(message.c_str());
}
