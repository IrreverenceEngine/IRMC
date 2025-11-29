#pragma once

#include <IRMC_CTypes.hpp>
#include <vector>

namespace IRMC {
    class Entity;

    struct MapStageInput {
        const std::vector<Entity>& ents;
        const AABB& bounds;
        const AABB& navbounds;
    };

    struct MapStageOutput {
        std::vector<char> navdata;
    };
}
