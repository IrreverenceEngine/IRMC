#include <IRMC_Face.hpp>
#include <IRMC_Macro.hpp>

namespace IRMC {

    Face::Face(const std::vector<glm::vec3>& vertices, const glm::vec3& normal, const char* texname, const glm::vec4& texu, const glm::vec4& texv, const glm::vec2& texscale)
    {
        m_Vertices.insert(m_Vertices.end(), vertices.begin(), vertices.end());
        m_Normal = normal;
        m_TexName = texname;

        glm::vec2 texSize = { 32, 32 };
        glm::vec2 texOffset = { texu.w, texv.w };
        glm::vec3 axisU = glm::vec3(texu.x, texu.y, texu.z) / texscale.x;
        glm::vec3 axisV = glm::vec3(texv.x, texv.y, texv.z) / texscale.y;

        for (const glm::vec3& vert : m_Vertices) {
            glm::vec2 texcoord = {
                vert.x * axisU.x + vert.y * axisU.y + vert.z * axisU.z,
                vert.x * axisV.x + vert.y * axisV.y + vert.z * axisV.z
            };

            texcoord += texOffset;
            texcoord /= texSize;

            m_Texcoords.push_back(texcoord);
        }
    }

}