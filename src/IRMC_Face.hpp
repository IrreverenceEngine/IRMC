#pragma once

#include <IRMC_Plane.hpp>
#include <IRMC_Macro.hpp>

#include <glm.hpp>

#include <vector>
#include <string>

namespace IRMC {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };

    struct Vec3Hash {
        UInt64 operator()(const glm::vec3& v) const {
            std::hash<UInt64> h;

            UInt64 hx = h(roundf(v.x * 1e6));
            UInt64 hy = h(roundf(v.y * 1e6));
            UInt64 hz = h(roundf(v.z * 1e6));
            return hx ^ (hy << 1) ^ (hz << 2);
        }
    };

    struct Vec3Equal {
        bool operator()(const glm::vec3& a, const glm::vec3& b) const IRMC_RETURN(a == b)
    };

    class Face {
    public:
        enum {
            FLAGS_NONE = 0,
            FLAGS_NOMESH = 1 << 0,      // Do not calculate Vertices.
            FLAGS_NORENDER = 1 << 1,    // Calculate Vertices but don't draw in Runtime.
            FLAGS_NOSHADOW = 1 << 2,    // Don't use this face for shadows in Runtime.
            FLAGS_UNWALKABLE = 1 << 3,  // Don't create NavMesh from this face.
        };

        Face(const std::vector<glm::vec3>& vertices,
            const Plane& plane,
            const char* texname,
            const glm::dvec4& texu,
            const glm::dvec4& texv,
            const glm::dvec2& texscale,
            UInt8 toolflags
        );

        Plane GetPlane() const IRMC_RETURN(m_Plane);
        glm::vec3 GetNormal() const IRMC_RETURN(m_Plane.normal)
        const std::vector<Vertex>& GetVertices() const IRMC_RETURN(m_Vertices)
        std::vector<Vertex>& GetVertices() IRMC_RETURN(m_Vertices)
        const std::vector<UInt32>& GetIndices() const IRMC_RETURN(m_Indices)

        const std::string& GetMaterialName() const IRMC_RETURN(m_TexName)
        UInt32 GetFlags() const IRMC_RETURN(m_Flags)
        AABB GetAABB() const IRMC_RETURN(m_AABB)

        private:
        Plane m_Plane;
        glm::vec2 m_Area;
        std::vector<Vertex> m_Vertices;
        std::vector<UInt32> m_Indices;

        std::string m_TexName;
        UInt32 m_Flags = FLAGS_NONE;
        AABB m_AABB = {};
    };
}
