#pragma once

#include <IRMC_Stage.hpp>
#include <IRMC_Macro.hpp>
#include <IRMC_Log.hpp>

namespace IRMC {
    class StageLightmap : public Stage {
    public:

        bool Run(const MapStageInput& in, MapStageOutput& out) override IRMC_UNIMPLEMENTED;

    };
}
