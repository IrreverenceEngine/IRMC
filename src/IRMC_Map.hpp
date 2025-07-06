#pragma once

#include <IRMC_Brush.hpp>
#include <IRMC_Entity.hpp>
#include <IRMC_CTypes.hpp>

#include <map>

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

        static const char* TypeName(MToken::Type type)
        {
            switch (type) {
            case MToken::Type::NUMBER: return "NUMBER";
            case MToken::Type::STRING: return "STRING";
            case MToken::Type::OPEN_CURLY: return "OPEN_CURLY";
            case MToken::Type::CLOSED_CURLY: return "CLOSED_CURLY";
            case MToken::Type::OPEN_ROUND: return "OPEN_ROUND";
            case MToken::Type::CLOSED_ROUND: return "CLOSED_ROUND";
            case MToken::Type::OPEN_SQUARE: return "OPEN_SQUARE";
            case MToken::Type::CLOSED_SQUARE: return "CLOSED_SQUARE";
            default: return "UNKNOWN";
            }
        }
    };



    class Map {
    public:
        Map(const char* mapdata);

        std::vector<Entity> m_Entities;

    private:
        void TknExpect(MToken::Type type);
        const MToken& TknPeek();
        const MToken& TknAdvance();
        bool TknIsEnd();

        void ParseEntity();
        void ParseBrush(Entity& ent);

        // Parsing
        std::vector<MToken> m_Tokens;
        UInt64 m_Pos = 0;
    };
}