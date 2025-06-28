#pragma once

#include <IRMC_CTypes.hpp>
#include <IRMC_Macro.hpp>

#include <glm.hpp>

#include <vector>
#include <string>

namespace IRMC {

    class Face {
    public:
        enum {
            FLAGS_NODRAW = 1 << 0,
        };

        Face(const std::vector<glm::vec3>& vertices, const glm::vec3& normal, const char* texname, const glm::highp_dvec4& texu, const glm::highp_dvec4& texv, const glm::highp_dvec2& texscale);

        const std::vector<glm::vec3>& GetVertices() const IRMC_RETURN(m_Vertices)
        const std::vector<glm::vec2>& GetTexcoords() const IRMC_RETURN(m_Texcoords)
        glm::vec3 GetNormal() const IRMC_RETURN(m_Normal)
        UInt32 GetFlags() const IRMC_RETURN(m_Flags)

    private:
        std::vector<glm::vec3> m_Vertices;
        std::vector<glm::vec2> m_Texcoords;
        glm::vec3 m_Normal;

        std::string m_TexName;
        UInt32 m_Flags;
    };
}