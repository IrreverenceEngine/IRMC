#pragma once

#include <IRMC_CTypes.hpp>

#include <glm.hpp>

namespace IRMC {
    struct Plane {
        glm::highp_dvec3 normal;
        Float64 dist;

        Float64 DistFromPoint(const glm::highp_dvec3& p) const;

        static Plane MakeFromPoints(const glm::highp_dvec3& a, const glm::highp_dvec3& b, const glm::highp_dvec3& c);
    };
}