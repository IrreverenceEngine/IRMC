#pragma once

#include <IRX_Common.hpp>
#include <IRMC_Stage.hpp>

namespace IRMC {
    class StageLightmap : public Stage {
    public:

        bool Run(const MapStageInput& in, MapStageOutput& out) override IRX_UNIMPLEMENTED;

    };
}
