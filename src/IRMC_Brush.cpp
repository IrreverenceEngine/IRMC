#include <IRMC_Brush.hpp>
#include <IRMC_QUtils.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_CTypes.hpp>

#include <cstdio>

namespace IRMC {

    constexpr glm::highp_dvec3 BOUNDS_VEC3 = glm::highp_dvec3(BOUNDS);
    constexpr Float64 EPSILON = 0.03125 / 4.0;

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

            if (da >= EPSILON) out.push_back(a);
            if (da * db < EPSILON) {
                Float64 t = da / (da - db);
                glm::highp_dvec3 intersection = a + (b - a) * t;
                out.push_back(intersection);
            }
        }

        return out;
    }

    static std::vector<glm::highp_dvec3> MakeBigQuad(const Plane& plane)
    {
        glm::highp_dvec3 normal = plane.normal;
        glm::highp_dvec3 right = (fabs(normal.z) > 0.99) ? glm::highp_dvec3(1, 0, 0) : glm::highp_dvec3(0, 0, 1);
        glm::highp_dvec3 up = glm::normalize(glm::cross(normal, right));
        right = glm::normalize(glm::cross(up, normal));

        glm::highp_dvec3 center = normal * -plane.dist;
        return {
            center + (right + up) * BOUNDS_VEC3,
            center + (right - up) * BOUNDS_VEC3,
            center + (-right - up) * BOUNDS_VEC3,
            center + (-right + up) * BOUNDS_VEC3
        };
    }

    Brush::Brush(const std::vector<Brushside>& brushsides)
    {
        for (UInt64 i = 0; i < brushsides.size(); i++) {
            Brushside brushside = brushsides[i];
            std::vector<glm::highp_dvec3> poly = MakeBigQuad(brushside.plane);

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
                for (size_t i = 1; i + 1 < poly.size(); i++) {
                    vertices.push_back(poly[0]);
                    vertices.push_back(poly[i]);
                    vertices.push_back(poly[i + 1]);
                }
            }

            m_Convex.reserve(poly.size());

            for (const auto& p : poly) {
                m_Convex.push_back(p);
            }

            for (UInt32 i = 0; i < m_Convex.size(); i++) { // TODO: Properly remove dupes
                for (UInt32 j = 0; j < m_Convex.size(); j++) {
                    if (m_Convex[i] == m_Convex[j]) {
                        m_Convex.erase(m_Convex.begin() + j);
                    }
                }
            }

            for (const auto& p : m_Convex) {
                IRMC_MSG(INFO, "%f, %f, %f", p.x, p.y, p.z);
            }

            Face face(
                vertices,
                brushside.plane,
                brushside.name.c_str(),
                brushside.texU,
                brushside.texV,
                brushside.texScale
            );

            m_Faces.push_back(std::move(face));
        }
    }
    
    std::vector<glm::vec3> Brush::GetTotalVertices() const
    {
        std::vector<glm::vec3> vertices;
        for (const Face& face : m_Faces) {
            const std::vector<glm::vec3>& faceVerts = face.GetVertices();
            vertices.insert(vertices.end(), faceVerts.begin(), faceVerts.end());
        }

        return vertices;
    }
    
    std::vector<glm::vec3> Brush::GetVisibleVertices() const
    {
        std::vector<glm::vec3> vertices;
        for (const Face& face : m_Faces) {
            if (face.GetFlags() & Face::FLAGS_NODRAW) {
                continue;
            }

            const std::vector<glm::vec3>& faceVerts = face.GetVertices();
            vertices.insert(vertices.end(), faceVerts.begin(), faceVerts.end());
        }

        return vertices;
    }

    std::vector<glm::vec3> Brush::GetTotalNormals() const
    {
        std::vector<glm::vec3> normals;
        for (const Face& face : m_Faces) {
            const glm::vec3& normal = face.GetNormal();

            for (UInt8 i = 0; i < 3; i++) {
                normals.push_back(normal);
            }
        }

        return normals;
    }
    
    std::vector<glm::vec3> Brush::GetVisibleNormals() const
    {
        std::vector<glm::vec3> normals;
        for (const Face& face : m_Faces) {
            if (face.GetFlags() & Face::FLAGS_NODRAW) {
                continue;
            }

            const glm::vec3& normal = face.GetNormal();

            for (UInt8 i = 0; i < 3; i++) {
                normals.push_back(normal);
            }
        }

        return normals;
    }

    std::vector<glm::vec2> Brush::GetVisibleTexcoords() const
    {
        std::vector<glm::vec2> texCoords;
        for (const Face& face : m_Faces) {
            if (face.GetFlags() & Face::FLAGS_NODRAW) {
                continue;
            }

            const std::vector<glm::vec2>& faceTexCoords = face.GetTexcoords();
            texCoords.insert(texCoords.end(), faceTexCoords.begin(), faceTexCoords.end());
        }

        return texCoords;
    }

}