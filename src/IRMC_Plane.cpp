#include <IRMC_Plane.hpp>

namespace IRMC {

    Float64 Plane::DistFromPoint(const glm::dvec3& p) const
    {
        return glm::dot(normal, p) + dist;
    }

    Float64 Plane::DistFromPlane(const Plane& other) const {
        if (!IsParallelTo(other)) {
            return INFINITY;
        }

        return fabs(dist - other.dist);
    }

    bool Plane::IsParallelTo(const Plane& other) const
    {
        glm::vec3 cross = glm::cross(normal, other.normal);
        return glm::length(cross) < 1e-6f;
    }

    bool Plane::IsFacingTo(const Plane& other) const
    {
        Float32 dot = glm::dot(normal, other.normal);
        return dot < 0.0;
    }

    Plane Plane::MakeFromPoints(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c)
    {
        Plane plane;
        plane.normal = glm::normalize(glm::cross(b - a, c - a));
        plane.dist = -glm::dot(plane.normal, a);

        return { plane.normal, plane.dist };
    }

}
