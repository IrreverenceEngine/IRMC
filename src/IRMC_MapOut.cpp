#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_QUtils.hpp>

#include <SDL3/SDL_endian.h>

#include <fstream>

namespace IRMC {

    constexpr float DOWNSCALE = 32.0f; // We have to make the world smoller ( Quake big :C )

    constexpr UInt32 MAGIC = 0x6D627269; // irbm
    constexpr UInt32 VERSION = 0;

    enum LumpInfoType {
        LUMPTYPE_ENTITIES,
        LUMPTYPE_BRUSHES,
        LUMPTYPE_FACES,
        LUMPTYPE_VERTICES,
        LUMPTYPE__COUNT
    };

    struct BMLumpInfo {
        UInt32 offset;
        UInt32 length;
    };

    struct BMHeader {
        UInt32 magic;
        UInt32 version;

        BMLumpInfo lumps[LUMPTYPE__COUNT];
    };

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

    void Map::CompileMap(const char* outpath)
    {
        std::ofstream stream(outpath, std::ios::binary);

        BMHeader header;
        header.magic = MAGIC;
        header.version = VERSION;

        WriteLE(stream, header.magic);
        WriteLE(stream, header.version);

        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            BMLumpInfo& lInfo = header.lumps[i];
            lInfo = { 0, 0 };

            WriteLE(stream, lInfo.offset);
            WriteLE(stream, lInfo.length);
        }

        WriteEntities(stream, header);
        WriteBrushes(stream, header);
        WriteFaces(stream, header);
        WriteVertices(stream, header);

        // Overwrite Lumps.
        stream.seekp(offsetof(BMHeader, lumps));
        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            BMLumpInfo& lInfo = header.lumps[i];
            WriteLE(stream, lInfo.offset);
            WriteLE(stream, lInfo.length);
        }
    }

    void Map::WriteEntities(std::ofstream& stream, BMHeader& header)
    {
        UInt32 startOff = stream.tellp();

        UInt32 brushOffset = 0;
        UInt32 brushCount = 0;
        for (const Entity& ent : m_Entities) {
            brushCount = ent.GetBrushes().size();

            const auto& kvs = ent.GetKeyValues();

            WriteLE(stream, (UInt32)kvs.size());

            for (const auto& [key, value] : kvs) {
                stream.write(key.c_str(), key.size() + 1);
                stream.write(value.c_str(), value.size() + 1);
            }

            WriteLE(stream, (UInt32)brushCount);
            WriteLE(stream, (UInt32)brushOffset);

            brushOffset += brushCount;
        }

        UInt32 endOff = stream.tellp();

        header.lumps[LUMPTYPE_ENTITIES] = { startOff, endOff - startOff };
    }

    void Map::WriteBrushes(std::ofstream& stream, BMHeader& header)
    {
        UInt32 startOff = stream.tellp();

        UInt32 faceOffset = 0;
        UInt32 faceCount = 0;
        for (const Entity& ent : m_Entities) {
            for (const Brush& brush : ent.GetBrushes()) {
                const auto& faces = brush.GetFaces();
                faceCount = faces.size();

                WriteLE(stream, (UInt32)faceCount);
                WriteLE(stream, (UInt32)faceOffset);

                // TODO: Terminate duplicate convex points. We should only have 8 points per cube
                WriteLE(stream, (UInt32)brush.GetConvex().size());
                for (const auto& point : brush.GetConvex()) {
                    WriteLE(stream, (Float32)point.x / DOWNSCALE);
                    WriteLE(stream, (Float32)point.y / DOWNSCALE);
                    WriteLE(stream, (Float32)point.z / DOWNSCALE);
                }

                faceOffset += faceCount;
            }
        }
        UInt32 endOff = stream.tellp();

        header.lumps[LUMPTYPE_BRUSHES] = { startOff, endOff - startOff };
    }

    void Map::WriteFaces(std::ofstream& stream, BMHeader& header)
    {
        UInt32 startOff = stream.tellp();

        UInt32 vertOffset = 0;
        for (const Entity& ent : m_Entities) {
            for (const Brush& brush : ent.GetBrushes()) {
                for (const Face& face : brush.GetFaces()) {
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
                }
            }
        }
        UInt32 endOff = stream.tellp();

        header.lumps[LUMPTYPE_FACES] = { startOff, endOff - startOff };
    }

    void Map::WriteVertices(std::ofstream& stream, BMHeader& header)
    {
        UInt32 startOff = stream.tellp();

        glm::vec3 tmpPos;
        glm::vec3 tmpNormal;
        glm::vec2 tmpUV;
        UInt32 cont = 0;
        for (const Entity& ent : m_Entities) {
            for (const Brush& brush : ent.GetBrushes()) {
                for (const Face& face : brush.GetFaces()) {
                    if (face.GetFlags() & Face::FLAGS_NODRAW) {
                        continue;
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
        }
        UInt32 endOff = stream.tellp();

        header.lumps[LUMPTYPE_VERTICES] = { startOff, endOff - startOff };
    }

}