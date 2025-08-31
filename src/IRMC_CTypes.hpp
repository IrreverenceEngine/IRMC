#pragma once

#include <IRMC_Macro.hpp>

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <glm.hpp>

namespace IRMC {
    using Int8 = int8_t;
    using UInt8 = uint8_t;
    using Int16 = int16_t;
    using UInt16 = uint16_t;
    using Int32 = int32_t;
    using UInt32 = uint32_t;
    using Int64 = int64_t;
    using UInt64 = uint64_t;
    using Float32 = float;
    using Float64 = double;

    struct AABB {
        glm::vec3 min = {};
        glm::vec3 max = {};

        glm::vec3 GetCenter() const IRMC_RETURN((max + min) / 2.0f)
        glm::vec3 GetHalfExtents() const IRMC_RETURN((max - min) / 2.0f)
        bool Intersects(const AABB& other) const IRMC_RETURN(
            (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z)
        )
    };
}
