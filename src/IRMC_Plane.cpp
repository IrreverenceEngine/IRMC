#include <IRMC_Plane.hpp>

namespace IRMC {

    Float64 Plane::DistFromPoint(const glm::highp_dvec3& p) const
    {
        return glm::dot(normal, p) + dist;
    }

    Plane Plane::MakeFromPoints(const glm::highp_dvec3& a, const glm::highp_dvec3& b, const glm::highp_dvec3& c)
    {
        Plane plane;
        plane.normal = glm::normalize(glm::cross(b - a, c - a));
        plane.dist = -glm::dot(plane.normal, a);

        return { plane.normal, plane.dist };
    }

}