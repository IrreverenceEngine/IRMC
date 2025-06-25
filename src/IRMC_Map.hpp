#pragma once

#include <IRMC_Brush.hpp>
#include <IRMC_Entity.hpp>
#include <IRMC_CTypes.hpp>

#include <string>

namespace IRMC {
    struct MToken {
        enum class Type {
            NUMBER,
            STRING,
            OPEN_CURLY,
            CLOSED_CURLY,
            OPEN_ROUND,
            CLOSED_ROUND,
            OPEN_SQUARE,
            CLOSED_SQUARE,
            END_OF_FILE
        } type;

        union {
            Float64 as_f64;
            char* as_str;
        } val;
    };

    class Map {
    public:
        Map(const char* mapdata);

        std::vector<Entity> m_Entities;

    private:
        bool TknExpect(MToken::Type type);
        const MToken& TknPeek();
        const MToken& TknAdvance();
        bool TknIsEnd();

        void ParseEntity();

        // Parsing
        std::vector<MToken> m_Tokens;
        UInt64 m_Pos = 0;

        // Entities
    };
}