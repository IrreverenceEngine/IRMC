#pragma once

#include <IRMC_Stage.hpp>

namespace IRMC {
    class StageNavmesh : public Stage {
    public:
        bool Run(const MapStageInput& in, MapStageOutput& out) override;
    };
}
