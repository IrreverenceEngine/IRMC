#pragma once

#include <IRX_Common.hpp>
#include <IRMC_MapStageIO.hpp>

namespace IRMC {

    class Stage {
    public:
        enum Level : UInt8 {
            LEVEL_NAVMESH,
            LEVEL_LIGHTMAP,
            _COUNT
        };

        static constexpr const char* NAMES[_COUNT] = {
            "Navmesh",
            "Lightmap"
        };

        virtual ~Stage() = default;
        virtual bool Run(const MapStageInput& in, MapStageOutput& out) = 0;
    };

}
