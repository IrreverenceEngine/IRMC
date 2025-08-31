#include <IRMC_Brush.hpp>
#include <IRMC_QUtils.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_CTypes.hpp>
#include <IRMC_Tools.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <cstdio>
#include <unordered_set>

namespace IRMC {

    constexpr glm::dvec3 BOUNDS_VEC3 = glm::dvec3(BOUNDS);
    constexpr Float64 EPSILON = 0.03125;

    static Float64 SnapFloat64(Float64 v) IRMC_RETURN(round(v / EPSILON) * EPSILON)
    static glm::dvec3 SnapVec3(const glm::dvec3& v) IRMC_RETURN(glm::dvec3(SnapFloat64(v.x), SnapFloat64(v.y), SnapFloat64(v.z)))

    static std::vector<glm::dvec3> ClipPolygon(const std::vector<glm::dvec3>& poly, const Plane& plane)
    {
        std::vector<glm::dvec3> out;

        for (UInt64 i = 0; i < poly.size(); i++) {
            glm::dvec3 a = poly[i];
            glm::dvec3 b = poly[(i + 1) % poly.size()];

            Float64 da = plane.DistFromPoint(a);
            Float64 db = plane.DistFromPoint(b);

            if (!std::isfinite(da)) {
                da = 0.0;
            }
            if (!std::isfinite(db)) {
                db = 0.0;
            }

            if (da >= EPSILON) {
                out.emplace_back(a);
            }

            if ((da > 0.0 && db < 0.0) || (da < 0.0 && db > 0.0) || (fabs(da) < EPSILON) || (fabs(db) < EPSILON)) {
                Float64 denom = da - db;
                if (fabs(denom) > 1e-12) {
                    Float64 t = da / denom;
                    glm::dvec3 intersection = a + (b - a) * t;

                    for (UInt32 j = 0; j < 3; j++) {
                        if (!std::isfinite(intersection[j])) {
                            intersection[j] = 0.5 * (a[j] + b[j]);
                        }
                    }

                    out.emplace_back(intersection);
                }
            }
        }

        return out;
    }

    static void MakeBigQuad(std::vector<glm::dvec3>& poly, const Plane& plane)
    {
        glm::dvec3 normal = plane.normal;
        glm::dvec3 right = (fabs(normal.z) > 0.99) ? glm::dvec3(1.0, 0, 0) : glm::dvec3(0, 0, 1.0);
        glm::dvec3 up = glm::normalize(glm::cross(normal, right));
        right = glm::normalize(glm::cross(up, normal));

        glm::dvec3 center = normal * -plane.dist;

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
            std::vector<glm::dvec3> poly;
            MakeBigQuad(poly, brushside.plane);

            for (UInt64 j = 0; j < brushsides.size(); j++) {
                if (i == j) {
                    continue;
                }

                const Brushside& otherBrushside = brushsides[j];

                poly = ClipPolygon(poly, otherBrushside.plane);
            }

            for (const glm::dvec3& p : poly) {
                uniquePoly.insert(p);
            }

            UInt8 tool = Tool::FromName(brushside.texname);

            UInt32 faceFlags = Tool::GetFaceFlags(tool);
            m_Flags |= Tool::GetBrushFlags(tool);

            if (faceFlags & Face::FLAGS_NOMESH) {
                Face face(
                    {},
                    brushside.plane,
                    brushside.texname.c_str(),
                    brushside.texU,
                    brushside.texV,
                    brushside.texScale,
                    faceFlags
                );

                m_Faces.emplace_back(face);
                continue;
            }

            for (glm::dvec3& p : poly) {
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

            Face face(
                vertices,
                brushside.plane,
                brushside.texname.c_str(),
                brushside.texU,
                brushside.texV,
                brushside.texScale,
                faceFlags
            );

            m_Faces.emplace_back(face);
        }

        // At the moment, we smoothe normals of faces in a single brush. However that is not enough.
        // We need to do this for all faces of

        m_Convex.reserve(uniquePoly.size());
        m_Convex.insert(m_Convex.end(), uniquePoly.begin(), uniquePoly.end());

        m_AABB = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
        for (const glm::vec3& p : m_Convex) {
            m_Origin += p;

            m_AABB.max = glm::max(p, m_AABB.max);
            m_AABB.min = glm::min(p, m_AABB.min);
        }
        m_Origin /= (Float32)m_Convex.size();

        for (glm::vec3& p : m_Convex) {
            p -= m_Origin;
        }

        if (m_Flags & FLAGS_NOCONVEX) {
            m_Convex.clear();
        }
    }

}
