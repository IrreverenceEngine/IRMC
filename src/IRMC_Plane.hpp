#pragma once

#include <glm.hpp>

namespace IRMC {
    struct Plane {
        glm::vec3 normal;
        float dist;

        float DistFromPoint(const glm::vec3& p) const;

        static Plane MakeFromPoints(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
    };
}