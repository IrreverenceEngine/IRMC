#pragma once

#include <IRMC_Brush.hpp>
#include <IRMC_Entity.hpp>
#include <IRMC_CTypes.hpp>


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
            CLOSED_SQUARE
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
        static constexpr float DOWNSCALE = 32.0f; // We have to make the world smoller ( Quake big :C )
        static constexpr UInt32 MAGIC = 0x6D627269; // irbm
        static constexpr UInt32 VERSION = 0;

        enum LumpInfoType {
            LUMPTYPE_ENTITIES,
            LUMPTYPE_BRUSHES,
            LUMPTYPE_FACES,
            LUMPTYPE_VERTICES,
            LUMPTYPE_MATERIALTABLE,
            LUMPTYPE__COUNT
        };

        // TODO: Remove these structs once the documentation is done

        struct BMLumpInfo {
            UInt32 offset;
            UInt32 length;
        };

        struct BMHeader {
            UInt32 magic = MAGIC;
            UInt32 version = VERSION;

            BMLumpInfo lumps[LUMPTYPE__COUNT];
        };

        void LoadMapFromData(const char* data);
        void LoadMapFromFile(const char* path);
        void CompileMap(const char* outpath);

        const std::vector<Entity>& GetEntities() const IRMC_RETURN(m_Entities);

    private:
        // Parsing
        void TknExpect(MToken::Type type);
        const MToken& TknPeek();
        const MToken& TknAdvance();
        bool TknIsEnd();

        void ParseEntity();
        void ParseBrush(Entity& ent);

        std::vector<MToken> m_Tokens;
        UInt64 m_Pos = 0;

        // Compiling
        void WriteEntities(std::vector<char>& stream);
        void WriteBrushes(std::vector<char>& stream);
        void WriteFaces(std::vector<char>& stream, std::map<std::string, UInt32>& matoffsets);
        void WriteVertices(std::vector<char>& stream);
        void WriteMaterialTable(std::vector<char>& stream, std::map<std::string, UInt32>& matoffsets);

        void WriteEntity(std::vector<char>& stream);
        void WriteBrush(std::vector<char>& stream);
        void WriteFace(std::vector<char>& stream, std::map<std::string, UInt32>& matoffsets);
        void WriteVertex(std::vector<char>& stream, const Face& face);

        std::vector<Entity> m_Entities;
        BMHeader m_Header;
    };

}