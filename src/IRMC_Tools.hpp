#pragma once

#include <IRMC_CTypes.hpp>

#include <glm.hpp>

#include <string>

namespace IRMC::Tool {

    enum {
        NODRAW, // TODO: I think we should rename this
        SKIP,
        AREA,
        TRIGGER,
        INVISIBLE,
        CLIP,
        SKY,
        _COUNT
    };

    UInt8 FromName(const std::string& texture);

    UInt32 GetBrushFlags(UInt8 tool);
    UInt32 GetFaceFlags(UInt8 tool);
}
