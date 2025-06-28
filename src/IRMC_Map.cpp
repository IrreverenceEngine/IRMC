#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_QUtils.hpp>

#include <ctype.h>

namespace IRMC {

    Map::Map(const char* mapdata)
    {
        // Tokenize

        const char* data = mapdata;
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
                case '{': { m_Tokens.push_back({ MToken::Type::OPEN_CURLY, 0 }); break; }
                case '}': { m_Tokens.push_back({ MToken::Type::CLOSED_CURLY, 0 }); break; }
                case '(': { m_Tokens.push_back({ MToken::Type::OPEN_ROUND, 0 }); break; }
                case ')': { m_Tokens.push_back({ MToken::Type::CLOSED_ROUND, 0 }); break; }
                case '[': { m_Tokens.push_back({ MToken::Type::OPEN_SQUARE, 0 }); break; }
                case ']': { m_Tokens.push_back({ MToken::Type::CLOSED_SQUARE, 0 }); break; }
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

                    MToken mtoken;
                    mtoken.type = MToken::Type::STRING;
                    mtoken.val.as_str = str;

                    m_Tokens.push_back(mtoken);

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

                MToken mtoken;
                mtoken.type = MToken::Type::STRING;
                mtoken.val.as_str = str;

                m_Tokens.push_back(mtoken);

            } else if (isdigit(c) || c == '-' || c == '.') {
                MToken mtoken;
                mtoken.type = MToken::Type::NUMBER;
                mtoken.val.as_f64 = strtod(data - 1, (char**)&data);

                m_Tokens.push_back(mtoken);
            }
        }

        // Parse Tokens

        while (!TknIsEnd()) {
            TknExpect(MToken::Type::OPEN_CURLY);
            ParseEntity();
            TknExpect(MToken::Type::CLOSED_CURLY);
        }

        for (const MToken& tkn : m_Tokens) {
            if (tkn.type == MToken::Type::STRING && tkn.val.as_str) {
                delete[] tkn.val.as_str;
            }
        }

        m_Tokens.clear();
    }
    
    void Map::TknExpect(MToken::Type type)
    {
        MToken::Type tkntype = TknPeek().type;
        if (tkntype != type) {
            IRMC_MSG(FATAL, "Expected %s, got %s", MToken::TypeName(type), MToken::TypeName(tkntype));
        }

        m_Pos++;
    }

    const MToken& Map::TknPeek() IRMC_RETURN(m_Tokens[m_Pos])
    const MToken& Map::TknAdvance() IRMC_RETURN(m_Tokens[m_Pos++])
    bool Map::TknIsEnd() IRMC_RETURN(m_Pos >= m_Tokens.size())

    void Map::ParseEntity()
    {
        Entity ent;
        while (!TknIsEnd() && TknPeek().type != MToken::Type::CLOSED_CURLY) {
            const MToken& tkn = TknPeek();

            if (tkn.type == MToken::Type::STRING) {
                std::string key = TknAdvance().val.as_str;
                std::string value = TknAdvance().val.as_str;

                ent.SetKeyValue(key, value);
            } else if (tkn.type == MToken::Type::OPEN_CURLY) {
                ParseBrush(ent);
            }
        }

        m_Entities.push_back(std::move(ent));
    }

    void Map::ParseBrush(Entity& ent)
    {
        TknExpect(MToken::Type::OPEN_CURLY);

        std::vector<Brushside> brushsides;
        while (!TknIsEnd() && TknPeek().type != MToken::Type::CLOSED_CURLY) {
            TknExpect(MToken::Type::OPEN_ROUND);
            glm::highp_dvec3 p1 = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(MToken::Type::CLOSED_ROUND);

            TknExpect(MToken::Type::OPEN_ROUND);
            glm::highp_dvec3 p2 = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(MToken::Type::CLOSED_ROUND);

            TknExpect(MToken::Type::OPEN_ROUND);
            glm::highp_dvec3 p3 = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(MToken::Type::CLOSED_ROUND);

            std::string texName = TknAdvance().val.as_str;

            TknExpect(MToken::Type::OPEN_SQUARE);
            glm::highp_dvec4 texU = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(MToken::Type::CLOSED_SQUARE);

            TknExpect(MToken::Type::OPEN_SQUARE);
            glm::highp_dvec4 texV = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };
            TknExpect(MToken::Type::CLOSED_SQUARE);

            Float64 rotation = TknAdvance().val.as_f64; // We don't use this.

            glm::highp_dvec2 texScale = {
                TknAdvance().val.as_f64,
                TknAdvance().val.as_f64
            };

            Brushside brushside {
                Plane::MakeFromPoints(QVec3ToVec3(p1),
                    QVec3ToVec3(p2),
                    QVec3ToVec3(p3)
                ),
                QVec4ToVec4(texU),
                QVec4ToVec4(texV),
                texScale,
                std::move(texName)
            };

            brushsides.push_back(std::move(brushside));
        }

        TknExpect(MToken::Type::CLOSED_CURLY);

        Brush brush(std::move(brushsides));
        ent.AddBrush(brush);
    }

}