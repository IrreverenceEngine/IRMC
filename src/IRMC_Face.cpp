#include <IRMC_Face.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>

#include <thirdparty/stb_image.h>

#include <map>
#include <cstring>

namespace IRMC {

    struct TextureInfo {
        Int32 width, height;
    };

    static std::map<std::string, TextureInfo> s_TextureInfos;

    Face::Face(const std::vector<glm::vec3>& vertices, const glm::vec3& normal, const char* texname, const glm::highp_dvec4& texu, const glm::highp_dvec4& texv, const glm::highp_dvec2& texscale)
    {
        m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
        m_Normal = normal;
        m_TexName = texname;
        m_Flags = 0;

        if (!strcmp(texname, "__NODRAW")) {
            IRMC_MSG(INFO, "NODRAW");
            m_Flags |= FLAGS_NODRAW;
        }

        if (!texname) {
            texname = "__ERROR";
        }

        if (!(m_Flags & FLAGS_NODRAW)) {
            TextureInfo texInfo;

            auto it = s_TextureInfos.find(texname);
            if (it == s_TextureInfos.end()) {
                Int32 _unused;
                if (!stbi_info(("bin/" + std::string(texname) + ".png").c_str(), &texInfo.width, &texInfo.height, &_unused)) {
                    IRMC_MSG(FATAL, "Could not find texture: %s", texname);
                }

                s_TextureInfos[texname] = texInfo;
            } else {
                texInfo = it->second;
            }

            glm::highp_dvec2 texSize = { texInfo.width, texInfo.height };
            glm::highp_dvec2 texOffset = { texu.w, texv.w };
            glm::highp_dvec3 axisU = glm::highp_dvec3(texu.x, texu.y, texu.z) / texscale.x;
            glm::highp_dvec3 axisV = glm::highp_dvec3(texv.x, texv.y, texv.z) / texscale.y;

            for (const glm::highp_dvec3& vert : m_Vertices) {
                glm::highp_dvec2 texcoord = {
                    vert.x * axisU.x + vert.y * axisU.y + vert.z * axisU.z,
                    vert.x * axisV.x + vert.y * axisV.y + vert.z * axisV.z
                };

                texcoord += texOffset;
                texcoord /= texSize;

                m_Texcoords.push_back(texcoord);
            }
        }

    }

}