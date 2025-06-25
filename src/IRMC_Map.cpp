#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_QUtils.hpp>

#include <ctype.h>

namespace IRMC {

    Map::Map(const char* mapdata)
    {
        // TOKENIZE

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

        m_Tokens.push_back({ MToken::Type::END_OF_FILE, 0 });

        // PARSE TOKENS
        // TODO: Rewrite this shit LMFAO

        while (!TknIsEnd()) {
            if (TknExpect(MToken::Type::OPEN_CURLY)) {
                TknAdvance();
                Entity ent;
                while (!TknIsEnd() && TknPeek().type != MToken::Type::CLOSED_CURLY) {
                    const MToken& fTkn = TknPeek();
                    if (fTkn.type == MToken::Type::STRING) {
                        // KeyValue
                        const MToken& key = TknAdvance();
                        const MToken& value = TknAdvance();
                        ent.SetKeyValue(key.val.as_str, value.val.as_str);
                    } else if (fTkn.type == MToken::Type::OPEN_CURLY) {
                        TknAdvance();
                        std::vector<Brushside> brushsides;

                        while (!TknIsEnd() && TknPeek().type != MToken::Type::CLOSED_CURLY) {
                            Brushside side;

                            TknAdvance();
                            glm::vec3 p1 = {
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64
                            };
                            TknAdvance();

                            TknAdvance();
                            glm::vec3 p2 = {
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64
                            };
                            TknAdvance();

                            TknAdvance();
                            glm::vec3 p3 = {
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64
                            };
                            TknAdvance();

                            const char* texname = TknAdvance().val.as_str;

                            TknAdvance();
                            glm::vec4 texU = {
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64
                            };
                            TknAdvance();

                            TknAdvance();
                            glm::vec4 texV = {
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64
                            };
                            TknAdvance();

                            Float64 rotation = TknAdvance().val.as_f64; // Skip Rotation, we don't use it.

                            glm::vec2 texScale = {
                                TknAdvance().val.as_f64,
                                TknAdvance().val.as_f64
                            };

                            side.plane = Plane::MakeFromPoints(QVec3ToVec3(p1), QVec3ToVec3(p2), QVec3ToVec3(p3));
                            side.texScale = texScale;
                            side.texU = QVec4ToVec4(texU);
                            side.texV = QVec4ToVec4(texV);

                            // IRMC_MSG(INFO,
                            //     "{\n(%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f) %s "
                            //     "[%.2f, %.2f, %.2f, %.2f] [%.2f, %.2f, %.2f, %.2f] %.2f %.2f %.2f\n}",
                            //     p1.x, p1.y, p1.z,
                            //     p2.x, p2.y, p2.z,
                            //     p3.x, p3.y, p3.z,
                            //     texname,
                            //     texU.x, texU.y, texU.z, texU.w,
                            //     texV.x, texV.y, texV.z, texV.w,
                            //     rotation, texScale.x, texScale.y
                            // );

                            brushsides.push_back(side);
                        }

                        ent.AddBrush(Brush(brushsides));
                    } else {
                        TknAdvance();
                    }

                }
                m_Entities.push_back(ent);
            }

            TknAdvance();
        }

        for (const MToken& tkn : m_Tokens) {
            if (tkn.type == MToken::Type::STRING && tkn.val.as_str) {
                delete[] tkn.val.as_str;
            }
        }

        m_Tokens.clear();
    }
    
    bool Map::TknExpect(MToken::Type type)
    {
        if (TknPeek().type != type) {
            TknAdvance();
            return false;
        }

        return true;
    }

    const MToken& Map::TknPeek() IRMC_RETURN(m_Tokens[m_Pos])
    const MToken& Map::TknAdvance() IRMC_RETURN(m_Tokens[m_Pos++])
    bool Map::TknIsEnd() IRMC_RETURN(m_Pos >= m_Tokens.size())

    void Map::ParseEntity()
    {
        Entity ent;
        while (!TknIsEnd() && TknPeek().type != MToken::Type::CLOSED_CURLY) {
            const MToken& fTkn = TknPeek();
            if (fTkn.type == MToken::Type::STRING) {
                const MToken& key = TknAdvance();
                const MToken& value = TknAdvance();
                IRMC_MSG(INFO, "%s", key.val.as_str);
                ent.SetKeyValue(key.val.as_str, value.val.as_str);
                m_Entities.push_back(ent);
            } else if (fTkn.type == MToken::Type::OPEN_CURLY) {
                TknAdvance();
                break;
            } else {
                TknAdvance();
            }
        }
    }
}