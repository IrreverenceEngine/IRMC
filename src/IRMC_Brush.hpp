#pragma once

#include <raylib.h>

#include <IRMC_Face.hpp>
#include <IRMC_Plane.hpp>

#include <vector>

namespace IRMC {

    constexpr float BOUNDS = 65536.0f; // min xyz: -65536, max xyz: 65536

    struct Brushside {
        Plane plane;
        glm::vec4 texU;
        glm::vec4 texV;
        glm::vec2 texScale;
    };

    class Brush {
    public:
        Brush(const std::vector<Brushside>& brushsides);

        std::vector<glm::vec3> GetTotalVertices() const;
        std::vector<glm::vec2> GetTotalTexcoords() const;
        std::vector<glm::vec3> GetTotalNormals() const;

        const std::vector<Brushside>& GetBrushside() const IRMC_RETURN(m_Brushsides)
        const std::vector<Face>& GetFaces() const IRMC_RETURN(m_Faces)

        void DebugDraw();

    private:
        std::vector<Brushside> m_Brushsides;
        std::vector<Face> m_Faces;
    };
}