#pragma once

#include <IRX_Common.hpp>

#include <glm.hpp>

namespace IRMC {
    struct Plane {
        glm::dvec3 normal;
        Float64 dist;

        Float64 DistFromPoint(const glm::dvec3& p) const;
        Float64 DistFromPlane(const Plane& p) const;
        bool IsParallelTo(const Plane& other) const;
        bool IsFacingTo(const Plane& other) const;

        static Plane MakeFromPoints(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c);
    };
}
