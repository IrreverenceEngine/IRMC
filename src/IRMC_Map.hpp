#pragma once

#include <IRX_Common.hpp>
#include <IRMC_Entity.hpp>
#include <IRMC_Face.hpp>
#include <IRMC_Stage.hpp>

namespace IRMC {

    class Map {
    public:
        static constexpr UInt32 MAGIC = 0x6D627269; // irbm
        static constexpr UInt32 VERSION = 0;

        enum LumpInfoType {
            LUMPTYPE_ENTITIES,
            LUMPTYPE_BRUSHES,
            LUMPTYPE_FACES,
            LUMPTYPE_VERTICES,
            LUMPTYPE_MATERIALTABLE,
            LUMPTYPE_NAVTILES,
            LUMPTYPE__COUNT
        };

        struct BMLumpInfo {
            UInt32 offset;
            UInt32 length;
        };

        struct BMHeader {
            UInt32 magic = MAGIC;
            UInt32 version = VERSION;
            UInt64 uncompressed_size; // If non-zero, the file is compressed.

            BMLumpInfo lumps[LUMPTYPE__COUNT];
        };

        ~Map();

        void EnableStage(Stage::Level stage);

        void LoadMapFromData(const char* data);
        void LoadMapFromFile(const char* path);
        void CompileMap(const char* outpath, bool compress);

        const std::vector<Entity>& GetEntities() const IRX_RETURN(m_Entities);

    private:
        struct Token {
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

            static const char* TypeName(Token::Type type)
            {
                switch (type) {
                case Token::Type::NUMBER: return "NUMBER";
                case Token::Type::STRING: return "STRING";
                case Token::Type::OPEN_CURLY: return "OPEN_CURLY";
                case Token::Type::CLOSED_CURLY: return "CLOSED_CURLY";
                case Token::Type::OPEN_ROUND: return "OPEN_ROUND";
                case Token::Type::CLOSED_ROUND: return "CLOSED_ROUND";
                case Token::Type::OPEN_SQUARE: return "OPEN_SQUARE";
                case Token::Type::CLOSED_SQUARE: return "CLOSED_SQUARE";
                default: return "UNKNOWN";
                }
            }
        };

        // Parsing
        std::vector<Token> m_Tokens;
        UInt64 m_Pos = 0;

        void Tokenize(const char* mdata);
        void Parse();
        void ParseEntity();
        void ParseBrush(Entity& ent);

        void TknExpect(Token::Type type);
        const Token& TknPeek() const IRX_RETURN(m_Tokens[m_Pos])
        const Token& TknAdvance() IRX_RETURN(m_Tokens[m_Pos++])
        bool TknIsEnd() const IRX_RETURN(m_Pos >= m_Tokens.size())

        // Writing
        BMHeader m_Header;

        void WriteMaterialTable(std::vector<char>& stream, std::map<std::string, UInt32>& matoffsets);
        void WriteEntity(std::vector<char>& stream, const Entity& ent);
        void WriteBrush(std::vector<char>& stream, const Brush& brush);
        void WriteFace(std::vector<char>& stream, const Face& face, std::map<std::string, UInt32>& matoffsets);
        void WriteVertex(std::vector<char>& stream, const Face& face);
        void WriteNavTiles(std::vector<char>& stream);

    private:
        Stage* m_Stages[Stage::_COUNT] = { nullptr };

        MapStageInput m_StageIn = { m_Entities, m_AABB, m_NavAABB };
        MapStageOutput m_StageOut = {};

        std::vector<Entity> m_Entities;
        AABB m_AABB = {};
        AABB m_NavAABB = {};
    };

}
