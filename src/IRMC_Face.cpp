#include <IRMC_Face.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_Game.hpp>

#include <thirdparty/stb_image.h>

#include <cstring>

namespace IRMC {

    Face::Face(const std::vector<glm::vec3>& vertices, const Plane& plane, const char* texname, const glm::highp_dvec4& texu, const glm::highp_dvec4& texv, const glm::highp_dvec2& texscale)
    {
        m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
        m_Plane = plane;
        m_TexName = texname;
        m_Flags = 0;

        if (!strcmp(texname, "NODRAW")) {
            m_Flags |= FLAGS_NODRAW;
        }

        if (!texname) {
            texname = "__ERROR";
        }

        if (!(m_Flags & FLAGS_NODRAW)) {
            Game::TextureInfo texInfo = Game::GetTextureInfo(texname);

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