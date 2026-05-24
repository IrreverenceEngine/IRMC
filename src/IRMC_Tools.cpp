#include <IRMC_Tools.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Face.hpp>

namespace IRMC::Tool {

    #define MAKE_TOOL_HASH(name) constexpr UInt64 HASH_##name = ToolHash(#name)

    constexpr UInt64 ToolHash(std::string_view str) {
        UInt64 hash = 1469598103934665603ull;
        for (char c : str) {
            hash ^= (UInt8)c;
            hash *= 1099511628211ull;
        }

        return hash;
    }

    MAKE_TOOL_HASH(NODRAW);
    MAKE_TOOL_HASH(SKIP);
    MAKE_TOOL_HASH(AREA);
    MAKE_TOOL_HASH(TRIGGER);
    MAKE_TOOL_HASH(INVISIBLE);
    MAKE_TOOL_HASH(CLIP);
    MAKE_TOOL_HASH(SKY);

    constexpr struct {
        UInt32 brushFlag;
        UInt32 faceFlag;
    } TOOL_INFOS[] = {
        /* NODRAW */    { Brush::FLAGS_NONE,                        Face::FLAGS_NOMESH | Face::FLAGS_UNWALKABLE },
        /* SKIP */      { Brush::FLAGS_NOCONVEX,                        Face::FLAGS_NOMESH | Face::FLAGS_UNWALKABLE },
        /* AREA */      { Brush::FLAGS_NOCONVEX | Brush::FLAGS_AREA,    Face::FLAGS_NOMESH | Face::FLAGS_UNWALKABLE },
        /* TRIGGER */   { Brush::FLAGS_NONE,                            Face::FLAGS_NOMESH | Face::FLAGS_UNWALKABLE },
        /* INVISIBLE */ { Brush::FLAGS_WALKABLE,                        Face::FLAGS_NORENDER },
        /* CLIP */      { Brush::FLAGS_WALKABLE,                        Face::FLAGS_NORENDER },
        /* SKY */       { Brush::FLAGS_NONE,                            Face::FLAGS_UNWALKABLE },

        /* DEFAULT */   { Brush::FLAGS_WALKABLE,                        Face::FLAGS_NONE }
    };

    UInt8 FromName(const std::string& texture)
    {
        switch (ToolHash(texture)) {
        case HASH_NODRAW: return NODRAW;
        case HASH_SKIP: return SKIP;
        case HASH_AREA: return AREA;
        case HASH_TRIGGER: return TRIGGER;
        case HASH_INVISIBLE: return INVISIBLE;
        case HASH_CLIP: return CLIP;
        case HASH_SKY: return SKY;
        default: return _COUNT;
        }
    }

    UInt32 GetBrushFlags(UInt8 tool) IRX_RETURN((tool <= _COUNT) ? TOOL_INFOS[tool].brushFlag : Brush::FLAGS_NONE)
    UInt32 GetFaceFlags(UInt8 tool) IRX_RETURN((tool <= _COUNT) ? TOOL_INFOS[tool].faceFlag : Face::FLAGS_NONE)

}
