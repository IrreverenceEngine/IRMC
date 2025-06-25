#include <IRMC_Brush.hpp>
#include <IRMC_QUtils.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_CTypes.hpp>

#include <cstdio>

namespace IRMC {

    static std::vector<glm::vec3> ClipPolygon(const std::vector<glm::vec3>& poly, const Plane& plane)
    {
        std::vector<glm::vec3> out;

        for (int i = 0; i < poly.size(); i++) {
            glm::vec3 a = poly[i];
            glm::vec3 b = poly[(i + 1) % poly.size()];

            float da = plane.DistFromPoint(a);
            float db = plane.DistFromPoint(b);

            if (da >= 0) out.push_back(a);
            if (da * db < 0) {
                float t = da / (da - db);
                glm::vec3 intersection = a + (b - a) * t;
                out.push_back(intersection);
            }
        }

        return out;
    }

    static std::vector<glm::vec3> MakeBigQuad(const Plane& plane)
    {
        glm::vec3 normal = plane.normal;
        glm::vec3 right = (fabs(normal.z) > 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1);
        glm::vec3 up = glm::normalize(glm::cross(normal, right));
        right = glm::normalize(glm::cross(up, normal));

        glm::vec3 center = normal * -plane.dist;
        return {
            center + (right + up) * BOUNDS,
            center + (right - up) * BOUNDS,
            center + (-right - up) * BOUNDS,
            center + (-right + up) * BOUNDS
        };
    }

    Brush::Brush(const std::vector<Brushside>& brushsides)
    {
        m_Brushsides.insert(m_Brushsides.end(), brushsides.begin(), brushsides.end());

        for (UInt64 i = 0; i < m_Brushsides.size(); i++) {
            Brushside brushside = m_Brushsides[i];
            std::vector<glm::vec3> poly = MakeBigQuad(brushside.plane);

            for (UInt64 j = 0; j < brushsides.size(); ++j) {
                if (i == j) {
                    continue;
                }

                poly = ClipPolygon(poly, m_Brushsides[j].plane);
            }

            std::vector<glm::vec3> vertices;
            if (poly.size() >= 3) {
                for (size_t i = 1; i + 1 < poly.size(); i++) {
                    vertices.push_back(poly[0]);
                    vertices.push_back(poly[i]);
                    vertices.push_back(poly[i + 1]);
                }

            }

            m_Faces.push_back(Face(
                vertices,
                brushside.plane.normal,
                "bruh",
                brushside.texU,
                brushside.texV,
                brushside.texScale
            ));
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

    std::vector<glm::vec2> Brush::GetTotalTexcoords() const
    {
        std::vector<glm::vec2> texCoords;
        for (const Face& face : m_Faces) {
            const std::vector<glm::vec2>& faceTexCoords = face.GetTexcoords();
            texCoords.insert(texCoords.end(), faceTexCoords.begin(), faceTexCoords.end());
        }

        return texCoords;
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

    void Brush::DebugDraw()
    {
        for (const Face& face : m_Faces) {
            const std::vector<glm::vec3>& verts = face.GetVertices();
            for (size_t i = 0; i < verts.size(); i += 3) {
                Vector3 v1 = {verts[i].x, verts[i].y, verts[i].z};
                Vector3 v2 = {verts[i+1].x, verts[i+1].y, verts[i+1].z};
                Vector3 v3 = {verts[i+2].x, verts[i+2].y, verts[i+2].z};

                DrawTriangle3D(v1, v2, v3, GRAY);
            }

            // Draw face normal as a line
            glm::vec3 center = (verts[0] + verts[1] + verts[2]) / 3.0f;
            glm::vec3 end = center + face.GetNormal() * 16.0f;
            DrawLine3D({center.x, center.y, center.z}, {end.x, end.y, end.z}, WHITE);
        }
    }
}