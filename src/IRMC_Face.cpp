#include <IRMC_Face.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_Game.hpp>
#include <IRMC_Map.hpp>

#include <Thirdparty/stb_image.h>

#include <cstring>
#include <unordered_map>

namespace IRMC {

    static glm::vec3 SurfaceOffset(const glm::vec3& v, const glm::vec3& normal, const glm::vec3& center, float offsetAmount) {
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
        const glm::dvec2& texscale,
        UInt8 toolflags
    )
    {
        m_Plane = plane;
        m_TexName = texname;
        m_Flags = toolflags;
        m_Plane.normal = glm::normalize(-m_Plane.normal);

        Float32 dPlaneUp = glm::dot(m_Plane.normal, {0, 1, 0});

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

            glm::vec3 center = {};

            for (const glm::vec3& vert : vertices) {
                center += vert;
            }

            center /= (Float32)vertices.size();

            Game::TextureInfo texInfo = Game::GetTextureInfo(texname);
            glm::dvec2 texSize = { texInfo.width, texInfo.height };
            glm::dvec3 axisU = glm::dvec3(texu.x, texu.y, texu.z) / texscale.x;
            glm::dvec3 axisV = glm::dvec3(texv.x, texv.y, texv.z) / texscale.y;
            glm::dvec2 texOffset = { texu.w, texv.w };

            for (const glm::vec3& vert : verts) {
                Float64 u = glm::dot((glm::dvec3)vert, axisU) + texOffset.x;
                Float64 v = glm::dot((glm::dvec3)vert, axisV) + texOffset.y;

                glm::dvec2 texcoord = glm::dvec2(u, v);
                texcoord /= texSize;
                texcoord.y = 1.0 - texcoord.y;

                Vertex vertex;
                vertex.position = SurfaceOffset(vert, -GetNormal(), center, 0.033f);
                vertex.normal = m_Plane.normal;
                vertex.texcoord = texcoord;

                m_Vertices.emplace_back(vertex);
            }
        }

        m_AABB = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
        for (const Vertex& vert : m_Vertices) {
            m_AABB.min = glm::min(m_AABB.min, vert.position);
            m_AABB.max = glm::max(m_AABB.max, vert.position);
        }
    }

}
