#include <IRMC_Face.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_Game.hpp>
#include <IRMC_Map.hpp>

#include <thirdparty/stb_image.h>

#include <cstring>
#include <unordered_map>

namespace IRMC {

    glm::vec3 SurfaceOffset(const glm::vec3& v, const glm::vec3& normal, const glm::vec3& center, float offsetAmount) {
        glm::vec3 v_offset = v - center;
        glm::vec3 tangent = v_offset - glm::dot(v_offset, normal) * normal;

        float tangentLen = glm::length(tangent);
        if (tangentLen < 1e-6f) {
            return v;
        }

        glm::vec3 direction = tangent / tangentLen;

        return v + direction * offsetAmount;
    }

    Face::Face(const std::vector<glm::vec3>& vertices,
        const Plane& plane, const char* texname,
        const glm::dvec4& texu,
        const glm::dvec4& texv,
        const glm::highp_dvec2& texscale
    )
    {
        m_Plane = plane;
        m_TexName = texname;

        m_Plane.normal = glm::normalize(-m_Plane.normal);

        if (!texname) {
            texname = "__ERROR";
        }

        if (texname && !strcmp(texname, "NODRAW")) {
            m_Flags |= FLAGS_NOMESH;
        }

        if (!(m_Flags & FLAGS_NOMESH)) {
            std::unordered_map<glm::vec3, UInt64, Vec3Hash, Vec3Equal> uniqueVerts;
            std::vector<glm::vec3> verts;

            for (const auto& v : vertices) {
                auto [it, inserted] = uniqueVerts.try_emplace(v, verts.size());
                if (inserted) {
                    verts.push_back(v);
                }

                m_Indices.emplace_back(it->second);
            }

            Game::TextureInfo texInfo = Game::GetTextureInfo(texname);

            glm::highp_dvec2 texSize = { texInfo.width, texInfo.height };
            glm::highp_dvec2 texOffset = { texu.w, texv.w };
            glm::highp_dvec3 axisU = glm::highp_dvec3(texu.x, texu.y, texu.z) / texscale.x;
            glm::highp_dvec3 axisV = glm::highp_dvec3(texv.x, texv.y, texv.z) / texscale.y;

            glm::vec3 center(0.0f);

            for (const glm::vec3& vert : vertices) {
                center += vert;
            }

            center /= static_cast<float>(vertices.size());

            for (const glm::vec3& vert : verts) {
                glm::vec2 texcoord = {
                    vert.x * axisU.x + vert.y * axisU.y + vert.z * axisU.z,
                    vert.x * axisV.x + vert.y * axisV.y + vert.z * axisV.z
                };

                texcoord += texOffset;
                texcoord /= texSize;

                Vertex vertex;
                vertex.position = SurfaceOffset(vert, -GetNormal(), center, 0.04f);
                vertex.normal = m_Plane.normal;
                vertex.texcoord = texcoord;

                m_Vertices.emplace_back(vertex);
            }
        }
    }

}
