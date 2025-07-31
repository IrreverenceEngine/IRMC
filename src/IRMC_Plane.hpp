#pragma once

#include <IRMC_CTypes.hpp>

#include <glm.hpp>

namespace IRMC {
    struct Plane {
        glm::dvec3 normal;
        Float64 dist;

        Float64 DistFromPoint(const glm::dvec3& p) const;

        static Plane MakeFromPoints(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c);
    };
}