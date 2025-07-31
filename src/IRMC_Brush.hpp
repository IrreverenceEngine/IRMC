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
        std::string name;
    };

    class Brush {
    public:
        enum {
            FLAGS_NONE = 0,
            FLAGS_NOCONVEX = 1 << 0,
        };

        Brush(const std::vector<Brushside>& brushsides);

        const std::vector<Face>& GetFaces() const IRMC_RETURN(m_Faces)
        const std::vector<glm::vec3>& GetConvex() const IRMC_RETURN(m_Convex)

    private:
        std::vector<Face> m_Faces;
        std::vector<glm::vec3> m_Convex; // For physics, just non-triangulated verts
        glm::vec3 m_Origin = {};
        UInt32 m_Flags = FLAGS_NONE;
    };
}