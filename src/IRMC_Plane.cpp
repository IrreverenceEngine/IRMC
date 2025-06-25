#include <IRMC_Plane.hpp>

namespace IRMC {

    float Plane::DistFromPoint(const glm::vec3& p) const
    {
        return glm::dot(normal, p) + dist;
    }

    Plane Plane::MakeFromPoints(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
    {
        Plane plane;
        plane.normal = glm::normalize(glm::cross(b - a, c - a));
        plane.dist = -glm::dot(plane.normal, a);

        return { plane.normal, plane.dist };
    }

}