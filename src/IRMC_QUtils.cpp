#include <IRMC_QUtils.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Defer.hpp>

namespace IRMC {

    glm::vec3 QVec3ToVec3(const glm::vec3& qvec3) IRMC_RETURN(glm::vec3(qvec3.x, qvec3.z, -qvec3.y))
    glm::vec4 QVec4ToVec4(const glm::vec4& qvec4) IRMC_RETURN(glm::vec4(qvec4.x, qvec4.z, -qvec4.y, qvec4.w))

    Float32 QStrToFloat32(const char* qstr) IRMC_RETURN(atof(qstr))

    Int32 QStrToInt32(const char* qstr) IRMC_RETURN(atoi(qstr))

    glm::vec3 QStrToVec3(const char* qstr)
    {
        Float32 vals[3] = { 0 };
        vals[0] = QStrToFloat32(qstr);
        UInt8 cntr = 1;
        for (UInt64 i = 0; i < strlen(qstr); i++) {
            if (cntr > 2) {
                break;
            }

            if (qstr[i] == ' ') {
                vals[cntr++] = QStrToFloat32(qstr + i + 1);
            }
        }

        return { vals[0], vals[1], vals[2] };
    }

}