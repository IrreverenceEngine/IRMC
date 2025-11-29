#pragma once

#include <IRMC_CTypes.hpp>
#include <IRMC_MapStageIO.hpp>

namespace IRMC {

    class Stage {
    public:
        enum {
            NAVMESH,
            LIGHTMAP,
            _COUNT
        };

        enum {
            NAVMESH_FLAG = 1 << NAVMESH,
            LIGHTMAP_FLAG = 1 << LIGHTMAP
        };

        static constexpr const char* NAMES[_COUNT] = {
            "Navmesh",
            "Lightmap"
        };

        virtual ~Stage() = default;
        virtual bool Run(const MapStageInput& in, MapStageOutput& out) = 0;
    };

}
