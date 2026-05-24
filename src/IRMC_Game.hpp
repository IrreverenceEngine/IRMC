#pragma once

#include <IRX_Common.hpp>

namespace IRMC::Game {
    struct TextureInfo {
        Int32 width, height;
    };

    void Init(const char* name, const char* path);

    const char* GetName();

    TextureInfo GetTextureInfo(const char* texname);
}