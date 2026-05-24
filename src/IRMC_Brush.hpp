#pragma once

#include <IRMC_Face.hpp>
#include <IRMC_Plane.hpp>

#include <vector>

namespace IRMC {

    class Map;

    constexpr float BOUNDS = 65536.0; // min xyz: -65536, max xyz: 65536

    struct Brushside {
        Plane plane;
        glm::dvec4 texU;
        glm::dvec4 texV;
        glm::dvec2 texScale;
        std::string texname;
    };

    class Brush {
    public:
        enum {
            FLAGS_NONE = 0,
            FLAGS_NOCONVEX = 1 << 0,
            FLAGS_WALKABLE = 1 << 1, // Has a walkable surface
            FLAGS_AREA = 1 << 2, // Is an area
        };

        Brush(const std::vector<Brushside>& brushsides);

        std::vector<Face>& GetFaces() IRX_RETURN(m_Faces)
        const std::vector<Face>& GetFaces() const IRX_RETURN(m_Faces)
        const std::vector<glm::vec3>& GetConvex() const IRX_RETURN(m_Convex)
        const glm::vec3& GetOrigin() const IRX_RETURN(m_Origin)
        const AABB& GetAABB() const IRX_RETURN(m_AABB)
        UInt32 GetFlags() const IRX_RETURN(m_Flags)

    private:
        void SmootheFaces();
    
        std::vector<Face> m_Faces;
        std::vector<glm::vec3> m_Convex; // For physics, just non-triangulated verts
        glm::vec3 m_Origin = {};
        AABB m_AABB = {};
        UInt32 m_Flags = FLAGS_NONE;
    };
}
