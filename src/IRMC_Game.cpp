#include <IRX_Common.hpp>
#include <IRMC_Game.hpp>

#include <Thirdparty/stb_image.h>

#include <map>
#include <string>

namespace IRMC::Game {

    static std::string s_Name;
    static std::string s_Path;

    static std::map<std::string, TextureInfo> s_TextureInfos;

    void Init(const char* name, const char* path)
    {
        s_Name = name;
        s_Path = path;
    }

    const char* GetName() IRX_RETURN(s_Name.c_str())

    TextureInfo GetTextureInfo(const char* texname)
    {
        TextureInfo texInfo = { 0 };

        if (!texname) {
            return texInfo;
        }

        auto it = s_TextureInfos.find(texname);
        if (it == s_TextureInfos.end()) {
            Int32 _unused;
            if (!stbi_info((s_Path + "/assets/textures/" + std::string(texname) + ".png").c_str(), &texInfo.width, &texInfo.height, &_unused)) {
                IRX_MSG(FATAL, "Could not find texture: %s", texname);
            }

            if ((texInfo.width % 2) != 0 || (texInfo.height % 2) != 0) {
                IRX_MSG(FATAL, "Texture dimensions should NOT be an odd number, \"%s\" (%d, %d)", texname, texInfo.width, texInfo.height);
            }

            s_TextureInfos[texname] = texInfo;
        } else {
            texInfo = it->second;
        }

        return texInfo;
    }

}
