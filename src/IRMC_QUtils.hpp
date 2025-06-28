#pragma once

#include <IRMC_CTypes.hpp>

#include <glm.hpp>

namespace IRMC {
    glm::highp_dvec3 QVec3ToVec3(const glm::highp_dvec3& qvec3); // Convert Quake (X,Z,-Y) to (X,Y,Z)
    glm::highp_dvec4 QVec4ToVec4(const glm::highp_dvec4& qvec4); // Convert Quake (X,Z,-Y,W) to (X,Y,Z,W)

    Float64 QStrToFloat64(const char* qstr);
    Int32 QStrToInt32(const char* qstr);
    glm::highp_dvec3 QStrToVec3(const char* qstr);
}