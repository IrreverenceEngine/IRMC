#include <IRX_Common.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>

#include <IRMC_StageNavmesh.hpp>
#include <IRMC_StageLightmap.hpp>

#include <ctype.h>
#include <cstring>

namespace IRMC {

    Map::~Map()
    {
        for (UInt8 i = 0; i < Stage::_COUNT; i++) {
            Stage* stage = m_Stages[i];
            if (stage) {
                delete stage;
                m_Stages[i] = nullptr;
            }
        }
    }

    void Map::EnableStage(Stage::Level stage)
    {
        Stage* stageobj = nullptr;
        switch (stage) {
        case Stage::LEVEL_NAVMESH: stageobj = new StageNavmesh; break;
        case Stage::LEVEL_LIGHTMAP: stageobj = new StageLightmap; break;
        default: return;
        };

        m_Stages[stage] = stageobj;
    }

    void Map::LoadMapFromData(const char* mdata)
    {
        Tokenize(mdata);
        Parse();

        m_AABB = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
        m_NavAABB = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
        for (const Entity& ents : m_Entities) {
            for (const Brush& brush : ents.GetBrushes()) {
                const AABB& aabb = brush.GetAABB();
                m_AABB.max = glm::max(aabb.max, m_AABB.max);
                m_AABB.min = glm::min(aabb.min, m_AABB.min);

                if (brush.GetFlags() & Brush::FLAGS_WALKABLE) {
                    m_NavAABB.max = glm::max(aabb.max, m_NavAABB.max);
                    m_NavAABB.min = glm::min(aabb.min, m_NavAABB.min);
                }
            }
        }
    }

    void Map::LoadMapFromFile(const char* path)
    {
        char* buffer = nullptr;
        IRX_DEFER({ if (buffer) delete[] buffer; });

        FILE* f = fopen(path, "rb");

        if (f) {
            UInt64 length = 0;

            fseek(f, 0, SEEK_END);
            length = ftell(f);
            fseek(f, 0, SEEK_SET);
            buffer = new char[length + 1];
            if (buffer) {
                fread(buffer, 1, length, f);
                buffer[length] = '\0';
            }

            fclose(f);
        } else {
            IRX_MSG(FATAL, "Couldn't find the map file to compile");
        }

        LoadMapFromData(buffer);
    }

    void Map::Tokenize(const char* mdata)
    {
        const char* data = mdata;
        while (char c = *data++) {
            if (c == ' ') {
                continue;
            }

            switch (c) {
                case '/': { // Skip Comment
                    char n = *data;

                    if (n == '/') {
                        do {
                            c = *data++;
                        } while (c != '\n');
                        continue;
                    }

                    break;
                }
                case '{': { m_Tokens.push_back({ Token::Type::OPEN_CURLY, 0 }); break; }
                case '}': { m_Tokens.push_back({ Token::Type::CLOSED_CURLY, 0 }); break; }
                case '(': { m_Tokens.push_back({ Token::Type::OPEN_ROUND, 0 }); break; }
                case ')': { m_Tokens.push_back({ Token::Type::CLOSED_ROUND, 0 }); break; }
                case '[': { m_Tokens.push_back({ Token::Type::OPEN_SQUARE, 0 }); break; }
                case ']': { m_Tokens.push_back({ Token::Type::CLOSED_SQUARE, 0 }); break; }
                case '"': {
                    const char* begin = data;

                    while ((c = *data)) {
                        if (c == '"') {
                            break;
                        }

                        data++;
                    }

                    const char* end = data++;
                    size_t size = end - begin;

                    char* str = new char[size + 1];
                    memcpy(str, begin, size);
                    str[size] = '\0';

                    Token mtoken;
                    mtoken.type = Token::Type::STRING;
                    mtoken.val.as_str = str;

                    m_Tokens.emplace_back(mtoken);

                    break;
                }
            }

            if (isalpha(c) || c == '_') {
                const char* begin = data - 1;

                while ((c = *data)) {
                    if (!isalnum(c) && c != '_') {
                        break;
                    }

                    data++;
                }

                const char* end = data;
                size_t size = end - begin;

                char* str = new char[size + 1];
                memcpy(str, begin, size);
                str[size] = '\0';

                Token mtoken;
                mtoken.type = Token::Type::STRING;
                mtoken.val.as_str = str;

                m_Tokens.emplace_back(mtoken);

            } else if (isdigit(c) || c == '-' || c == '.') {
                Token mtoken;
                mtoken.type = Token::Type::NUMBER;
                mtoken.val.as_f64 = strtod(data - 1, (char**)&data);

                m_Tokens.emplace_back(mtoken);
            }
        }
    }
    
    void Map::Parse()
    {
        while (!TknIsEnd()) {
            TknExpect(Token::Type::OPEN_CURLY);
            ParseEntity();
            TknExpect(Token::Type::CLOSED_CURLY);
        }

        for (const Token& tkn : m_Tokens) {
            if (tkn.type == Token::Type::STRING && tkn.val.as_str) {
                delete[] tkn.val.as_str;
            }
        }

        m_Tokens.clear();
    }

    void Map::ParseEntity()
    {
        Entity ent;
        while (!TknIsEnd() && TknPeek().type != Token::Type::CLOSED_CURLY) {
            const Token& tkn = TknPeek();

            if (tkn.type == Token::Type::STRING) {
                std::string key = TknAdvance().val.as_str;
                std::string value = TknAdvance().val.as_str;

                ent.SetKeyValue(key, value);
            } else if (tkn.type == Token::Type::OPEN_CURLY) {
                ParseBrush(ent);
            }
        }

        m_Entities.emplace_back(ent);
    }

    void Map::ParseBrush(Entity& ent)
    {
        TknExpect(Token::Type::OPEN_CURLY);

        std::vector<Brushside> brushsides;
        while (!TknIsEnd() && TknPeek().type != Token::Type::CLOSED_CURLY) {
            TknExpect(Token::Type::OPEN_ROUND);
            glm::dvec3 p1 = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(Token::Type::CLOSED_ROUND);

            TknExpect(Token::Type::OPEN_ROUND);
            glm::dvec3 p2 = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(Token::Type::CLOSED_ROUND);

            TknExpect(Token::Type::OPEN_ROUND);
            glm::dvec3 p3 = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(Token::Type::CLOSED_ROUND);

            std::string texName = TknAdvance().val.as_str;

            TknExpect(Token::Type::OPEN_SQUARE);
            glm::dvec4 texU = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(Token::Type::CLOSED_SQUARE);

            TknExpect(Token::Type::OPEN_SQUARE);
            glm::dvec4 texV = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(Token::Type::CLOSED_SQUARE);

            Float64 rotation = TknAdvance().val.as_f64; // We don't use this.

            glm::dvec2 texScale = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };

            Brushside brushside {
                Plane::MakeFromPoints(
                    QUtil::QVec3ToVec3(p1),
                    QUtil::QVec3ToVec3(p2),
                    QUtil::QVec3ToVec3(p3)
                ),
                QUtil::QVec4ToVec4(texU),
                QUtil::QVec4ToVec4(texV),
                texScale,
                texName
            };

            brushsides.emplace_back(brushside);
        }

        TknExpect(Token::Type::CLOSED_CURLY);

        Brush brush(brushsides);
        ent.AddBrush(brush);
    }

    void Map::TknExpect(Token::Type type)
    {
        Token::Type tkntype = TknPeek().type;
        if (tkntype != type) {
            IRX_MSG(FATAL, "Expected %s, got %s", Token::TypeName(type), Token::TypeName(tkntype));
        }

        m_Pos++;
    }

}
