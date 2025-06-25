#pragma once

#include <IRMC_CTypes.hpp>

#include <glm.hpp>

namespace IRMC {
    glm::vec3 QVec3ToVec3(const glm::vec3& qvec3); // Convert Quake (X,Z,-Y) to (X,Y,Z)
    glm::vec4 QVec4ToVec4(const glm::vec4& qvec4); // Convert Quake (X,Z,-Y,W) to (X,Y,Z,W)

    Float32 QStrToFloat32(const char* qstr);
    Int32 QStrToInt32(const char* qstr);
    glm::vec3 QStrToVec3(const char* qstr);
}