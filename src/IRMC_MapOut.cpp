#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_QUtils.hpp>

#include <SDL3/SDL_endian.h>

#include <vector>
#include <fstream>

namespace IRMC {

    // TODO: Remove these structs once the documentation is done

    struct BMEntity {
        UInt32 kvNum; // Number of KV pairs, used to know when to stop reading
        char* kv; // includes two null, example: myKey\0myValue\0
        UInt32 brushNum; // number of brushes to read after first
        UInt32 brushBegin; // first brush
    };

    struct BMLumpEntities {
        BMEntity* entities;
    };

    struct BMBrush {
        UInt32 faceNum;
        UInt32 faceBegin;
        UInt32 convexPointsNum;
        Float32 convexPoints;
    };

    struct BMLumpBrushes {
        BMBrush* brushes;
    };

    struct BMPlane {
        Float32 nx, ny, nz;
        Float32 d;
    };

    struct BMFace {
        BMPlane plane;
        UInt32 flags;
        UInt32 vertNum;
        UInt32 vertBegin;
    };

    struct BMLumpFaces {
        BMFace* faces;
    };

    struct BMVertex {
        Float32 px, py, pz;
        Float32 nx, ny, nz;
        Float32 tx, ty;
    };

    struct BMLumpVertices {
        BMVertex* faces;
    };

    template<typename T>
    static void WriteLE(std::vector<char>& stream, T value)
    {
        if constexpr (sizeof(T) == 4) {
            UInt32 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt32 v = SDL_Swap32LE(tmp);
            char* vp = (char*)&v;
            stream.reserve(stream.size() + sizeof(v));
            stream.insert(stream.end(), vp, vp + sizeof(v));
        } else if constexpr (sizeof(T) == 8) {
            UInt64 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt64 v = SDL_Swap64LE(tmp);
            char* vp = (char*)&v;
            stream.reserve(stream.size() + sizeof(v));
            stream.insert(stream.end(), vp, vp + sizeof(v));
        } else if constexpr (sizeof(T) == 2) {
            UInt16 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt16 v = SDL_Swap16LE(tmp);
            char* vp = (char*)&v;
            stream.reserve(stream.size() + sizeof(v));
            stream.insert(stream.end(), vp, vp + sizeof(v));
        } else {
            char* vp = (char*)&value;
            stream.insert(stream.end(), vp, vp + sizeof(value));
        }
    }

    template<typename T>
    static void WriteLE(std::ostream& stream, T value) {
        if constexpr (sizeof(T) == 4) {
            UInt32 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt32 v = SDL_Swap32LE(tmp);
            stream.write((const char*)(&v), sizeof(v));
        } else if constexpr (sizeof(T) == 8) {
            UInt64 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt64 v = SDL_Swap64LE(tmp);
            stream.write((const char*)(&v), sizeof(v));
        } else if constexpr (sizeof(T) == 2) {
            UInt16 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt16 v = SDL_Swap16LE(tmp);
            stream.write((const char*)(&v), sizeof(v));
        } else {
            stream.write((const char*)(&value), sizeof(value));
        }
    }

    static void Write(std::vector<char>& stream, const char* beg, UInt64 len)
    {
        stream.insert(stream.end(), beg, beg + len);
    }

    static void Write(std::ostream& stream, const char* beg, UInt64 len)
    {
        stream.write(beg, len);
    }

    void Map::CompileMap(const char* outpath)
    {
        std::map<std::string, UInt32> matOffsets;

        std::vector<char> streams[LUMPTYPE__COUNT];
        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            streams[i].reserve(32768);
        }

        WriteMaterialTable(streams[LUMPTYPE_MATERIALTABLE], matOffsets);

        for (const Entity& ent : m_Entities) {
            WriteEntity(streams[LUMPTYPE_ENTITIES], ent);

            for (const Brush& brush : ent.GetBrushes()) {
                WriteBrush(streams[LUMPTYPE_BRUSHES], brush);

                for (const Face& face : brush.GetFaces()) {
                    WriteFace(streams[LUMPTYPE_FACES], matOffsets, face);
                    WriteVertex(streams[LUMPTYPE_VERTICES], face);
                }
            }
        }

        std::ofstream outStream(outpath, std::ios::binary);
        WriteLE(outStream, m_Header.magic);
        WriteLE(outStream, m_Header.version);

        UInt32 offset = sizeof(m_Header);
        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            UInt32 streamLen = streams[i].size();
            WriteLE(outStream, offset);
            WriteLE(outStream, streamLen);

            offset += streamLen;
        }

        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            std::vector<char>& lumpStr = streams[i];
            outStream.write(lumpStr.data(), lumpStr.size());
        }
    }

    void Map::WriteMaterialTable(std::vector<char>& stream, std::map<std::string, UInt32>& matoffsets)
    {
        for (const Entity& ent : m_Entities) {
            for (const Brush& brush : ent.GetBrushes()) {
                for (const Face& face : brush.GetFaces()) {
                    if (face.GetFlags() & Face::FLAGS_NODRAW) {
                        continue;
                    }

                    const std::string& matName = face.GetMaterialName();
                    if (matoffsets.find(matName) == matoffsets.end()) {
                        if (matName.size() > UINT8_MAX) {
                            IRMC_MSG(FATAL, "Material Name cannot be longer than 255");
                        }

                        matoffsets[matName] = stream.size();
                        WriteLE(stream, (UInt8)matName.size());
                        Write(stream, matName.c_str(), matName.size());
                    }
                }
            }
        }
    }

    void Map::WriteEntity(std::vector<char>& stream, const Entity& ent)
    {
        static UInt32 brushOffset = 0;
        static UInt32 brushCount = 0;

        brushCount = ent.GetBrushes().size();

        const auto& kvs = ent.GetKeyValues();

        WriteLE(stream, (UInt32)kvs.size());

        for (const auto& [key, value] : kvs) {
            Write(stream, (char*)key.c_str(), key.size() + 1);
            Write(stream, (char*)value.c_str(), value.size() + 1);
        }

        WriteLE(stream, (UInt32)brushCount);
        WriteLE(stream, (UInt32)brushOffset);

        brushOffset += brushCount;
    }

    void Map::WriteBrush(std::vector<char>& stream, const Brush& brush)
    {
        static UInt32 faceOffset = 0;
        static UInt32 faceCount = 0;

        faceCount = brush.GetFaces().size();

        WriteLE(stream, (UInt32)faceCount);
        WriteLE(stream, (UInt32)faceOffset);

        WriteLE(stream, (UInt32)brush.GetConvex().size());
        for (const auto& point : brush.GetConvex()) {
            WriteLE(stream, (Float32)point.x / DOWNSCALE);
            WriteLE(stream, (Float32)point.y / DOWNSCALE);
            WriteLE(stream, (Float32)point.z / DOWNSCALE);
        }

        faceOffset += faceCount;
    }

    void Map::WriteFace(std::vector<char>& stream, std::map<std::string, UInt32>& matoffsets, const Face& face)
    {
        static UInt32 vertOffset = 0;

        const Plane& plane = face.GetPlane();

        WriteLE(stream, (Float32)plane.normal.x);
        WriteLE(stream, (Float32)plane.normal.y);
        WriteLE(stream, (Float32)plane.normal.z);
        WriteLE(stream, (Float32)plane.dist);

        WriteLE(stream, (UInt32)face.GetFlags());

        if (!(face.GetFlags() & Face::FLAGS_NODRAW)) {
            WriteLE(stream, (UInt32)face.GetVertices().size());
            WriteLE(stream, (UInt32)vertOffset);

            vertOffset += face.GetVertices().size();
        } else {
            WriteLE(stream, (UInt32)0);
            WriteLE(stream, (UInt32)vertOffset);
        }

        WriteLE(stream, (UInt32)matoffsets[face.GetMaterialName()]);
    }
    
    void Map::WriteVertex(std::vector<char>& stream, const Face& face)
    {
        static glm::vec3 tmpPos;
        static glm::vec3 tmpNormal;
        static glm::vec2 tmpUV;
        static UInt32 cont = 0;

        if (face.GetFlags() & Face::FLAGS_NODRAW) {
            return;
        }

        for (UInt32 i = 0; i < face.GetVertices().size(); i++) {
            tmpPos = face.GetVertices().at(i) / DOWNSCALE;
            tmpNormal = face.GetNormal();
            tmpUV = face.GetTexcoords().at(i);

            WriteLE(stream, tmpPos.x);
            WriteLE(stream, tmpPos.y);
            WriteLE(stream, tmpPos.z);

            WriteLE(stream, tmpNormal.x);
            WriteLE(stream, tmpNormal.y);
            WriteLE(stream, tmpNormal.z);

            WriteLE(stream, tmpUV.x);
            WriteLE(stream, tmpUV.y);
        }
    }
    
}