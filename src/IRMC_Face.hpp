#pragma once

#include <IRMC_Macro.hpp>

#include <glm.hpp>

#include <vector>
#include <string>

namespace IRMC {
    class Face {
    public:
        Face(const std::vector<glm::vec3>& vertices, const glm::vec3& normal, const char* texname, const glm::vec4& texu, const glm::vec4& texv, const glm::vec2& texscale);

        const std::vector<glm::vec3>& GetVertices() const IRMC_RETURN(m_Vertices)
        const std::vector<glm::vec2>& GetTexcoords() const IRMC_RETURN(m_Texcoords)
        glm::vec3 GetNormal() const IRMC_RETURN(m_Normal)

    private:
        std::vector<glm::vec3> m_Vertices;
        std::vector<glm::vec2> m_Texcoords;
        glm::vec3 m_Normal;

        std::string m_TexName;
    };
}