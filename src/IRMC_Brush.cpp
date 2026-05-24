#include <IRX_Common.hpp>

#include <IRMC_Brush.hpp>
#include <IRMC_Tools.hpp>

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <unordered_set>

namespace IRMC {

    constexpr glm::dvec3 BOUNDS_VEC3 = glm::dvec3(BOUNDS);
    constexpr Float64 EPSILON = 0.03125;

    static Float64 SnapFloat64(Float64 v) IRX_RETURN(round(v / EPSILON) * EPSILON)
    static glm::dvec3 SnapVec3(const glm::dvec3& v) IRX_RETURN(glm::dvec3(SnapFloat64(v.x), SnapFloat64(v.y), SnapFloat64(v.z)))

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

        SmootheFaces();
    }

    void Brush::SmootheFaces()
    {
        struct VertexRef {
            Face* face;
            glm::vec3 weightedNorm;
            uint32_t id;
        };

        std::unordered_map<glm::vec3, std::vector<VertexRef>, Vec3Hash, Vec3Equal> vertexlist;

        for (Face& face : m_Faces) {
            // WARN: Why the FUCK can there be a face with less than 3 vertices. Need to look into it.
            if ((face.GetFlags() & Face::FLAGS_NOMESH) || (face.GetFlags() & Face::FLAGS_NORENDER) || face.GetVertices().size() < 3) {
                continue;
            }
            
            const glm::vec3& v0 = face.GetVertices()[0].position;
            const glm::vec3& v1 = face.GetVertices()[1].position;
            const glm::vec3& v2 = face.GetVertices()[2].position;
            const glm::vec3 weightedNorm = face.GetNormal()/* glm::cross(v1 - v0, v2 - v0) */;

            for (UInt32 i = 0; i < face.GetVertices().size(); i++) {
                const Vertex& vert = face.GetVertices()[i];
                vertexlist[vert.position].push_back({ &face, weightedNorm, i });
            }
        }

        float maxAngle = 60.0f;
        float minDot = cos(glm::radians(maxAngle));

        // The system here is complicated asf so let me TLDR a bit
        // Basically, we average over the faces that are facing the correct direction. We do this by removing majority of the angle check failures. After normalizing the sum of those values, we get the desired result.
        for (auto& kv : vertexlist) {
            auto& refs = kv.second;

            if (refs.size() == 1) {
                continue;
            }

            for (VertexRef& refA : refs) {
                glm::vec3 accum = {};
                glm::vec3 normalA = refA.face->GetNormal();

                for (VertexRef& refB : refs) {
                    glm::vec3 normalB = refB.face->GetNormal();

                    if (glm::dot(normalA, normalB) >= minDot) {
                        accum += refB.weightedNorm;
                    }
                }

                refA.face->GetVertices()[refA.id].normal = glm::normalize(accum);
            }
        }
    }

}
