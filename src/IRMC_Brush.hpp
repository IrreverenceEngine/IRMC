#pragma once

#include <IRMC_Face.hpp>
#include <IRMC_Plane.hpp>

#include <vector>

namespace IRMC {

    constexpr float BOUNDS = 65536.0; // min xyz: -65536, max xyz: 65536

    struct Brushside {
        Plane plane;
        glm::highp_dvec4 texU;
        glm::highp_dvec4 texV;
        glm::highp_dvec2 texScale;
        std::string name;
    };

    class Brush {
    public:
        Brush(const std::vector<Brushside>& brushsides);

        std::vector<glm::vec3> GetTotalVertices() const;
        std::vector<glm::vec3> GetVisibleVertices() const;
        std::vector<glm::vec3> GetTotalNormals() const;
        std::vector<glm::vec3> GetVisibleNormals() const;
        std::vector<glm::vec2> GetVisibleTexcoords() const;

        const std::vector<Face>& GetFaces() const IRMC_RETURN(m_Faces)
        const std::vector<glm::vec3>& GetConvex() const IRMC_RETURN(m_Convex)

    private:
        std::vector<Face> m_Faces;
        std::vector<glm::vec3> m_Convex; // For physics, just non-triangulated verts
    };
}