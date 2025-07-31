#include <IRMC_Brush.hpp>
#include <IRMC_QUtils.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_CTypes.hpp>

#include <cstdio>

#include <unordered_set>

namespace IRMC {

    constexpr glm::highp_dvec3 BOUNDS_VEC3 = glm::highp_dvec3(BOUNDS);
    constexpr Float64 EPSILON = 0.125;

    static Float64 SnapFloat64(Float64 v) IRMC_RETURN(round(v / EPSILON) * EPSILON)
    static glm::highp_dvec3 SnapVec3(const glm::highp_dvec3& v) IRMC_RETURN(glm::highp_dvec3(SnapFloat64(v.x), SnapFloat64(v.y), SnapFloat64(v.z)))

    static std::vector<glm::highp_dvec3> ClipPolygon(const std::vector<glm::highp_dvec3>& poly, const Plane& plane)
    {
        std::vector<glm::highp_dvec3> out;

        for (UInt32 i = 0; i < poly.size(); i++) {
            glm::highp_dvec3 a = poly[i];
            glm::highp_dvec3 b = poly[(i + 1) % poly.size()];

            Float64 da = plane.DistFromPoint(a);
            Float64 db = plane.DistFromPoint(b);

            if (da >= EPSILON) {
                out.emplace_back(a);
            }

            if (da * db < EPSILON) {
                Float64 t = da / (da - db);
                glm::highp_dvec3 intersection = a + (b - a) * t;
                out.emplace_back(intersection);
            }
        }

        return out;
    }

    static void MakeBigQuad(std::vector<glm::highp_dvec3>& poly, const Plane& plane)
    {
        glm::highp_dvec3 normal = plane.normal;
        glm::highp_dvec3 right = (fabs(normal.z) > 0.99) ? glm::highp_dvec3(1.0, 0, 0) : glm::highp_dvec3(0, 0, 1.0);
        glm::highp_dvec3 up = glm::normalize(glm::cross(normal, right));
        right = glm::normalize(glm::cross(up, normal));

        glm::highp_dvec3 center = normal * -plane.dist;

        poly.reserve(4);
        poly.emplace_back(center + (right + up) * BOUNDS_VEC3);
        poly.emplace_back(center + (right - up) * BOUNDS_VEC3);
        poly.emplace_back(center + (-right - up) * BOUNDS_VEC3);
        poly.emplace_back(center + (-right + up) * BOUNDS_VEC3);
    }

    Brush::Brush(const std::vector<Brushside>& brushsides)
    {
        std::unordered_set<glm::vec3, Vec3Hash, Vec3Equal> uniquePoly;

        for (UInt64 i = 0; i < brushsides.size(); i++) {
            Brushside brushside = brushsides[i];
            std::vector<glm::highp_dvec3> poly;
            MakeBigQuad(poly, brushside.plane);

            for (UInt64 j = 0; j < brushsides.size(); j++) {
                if (i == j) {
                    continue;
                }

                poly = ClipPolygon(poly, brushsides[j].plane);
            }

            for (glm::highp_dvec3& p : poly) {
                p = SnapVec3(p);
            }

            std::vector<glm::vec3> vertices;
            if (poly.size() >= 3) {
                vertices.reserve(poly.size() * 3);
                for (UInt64 i = 1; i + 1 < poly.size(); i++) {
                    vertices.emplace_back(poly[0]);
                    vertices.emplace_back(poly[i]);
                    vertices.emplace_back(poly[i + 1]);
                }
            }

            for (const glm::dvec3& p : poly) {
                uniquePoly.insert(p);
            }

            Face face(
                vertices,
                brushside.plane,
                brushside.name.c_str(),
                brushside.texU,
                brushside.texV,
                brushside.texScale
            );

            m_Faces.emplace_back(face);
        }

        m_Convex.reserve(uniquePoly.size());
        m_Convex.insert(m_Convex.end(), uniquePoly.begin(), uniquePoly.end());

        for (const glm::vec3& p : m_Convex) {
            m_Origin += p;
        }
        m_Origin /= m_Convex.size();
    }

}