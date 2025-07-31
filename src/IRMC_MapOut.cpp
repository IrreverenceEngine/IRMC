#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_QUtils.hpp>

#include <SDL3/SDL_endian.h>

#include <vector>
#include <fstream>

namespace IRMC {

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
                    WriteFace(streams[LUMPTYPE_FACES], face, matOffsets);
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
                    if (face.GetFlags() & Face::FLAGS_NOMESH) {
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

    void Map::WriteFace(std::vector<char>& stream, const Face& face, std::map<std::string, UInt32>& matoffsets)
    {
        static UInt32 vertOffset = 0;

        const Plane& plane = face.GetPlane();

        WriteLE(stream, (Float32)plane.normal.x);
        WriteLE(stream, (Float32)plane.normal.y);
        WriteLE(stream, (Float32)plane.normal.z);
        WriteLE(stream, (Float32)plane.dist);

        WriteLE(stream, (UInt32)face.GetFlags());
        WriteLE(stream, (UInt32)matoffsets[face.GetMaterialName()]);

        if (face.GetFlags() & Face::FLAGS_NOMESH) {
            WriteLE(stream, (UInt32)0);
            WriteLE(stream, (UInt32)vertOffset);
            WriteLE(stream, 0);
        } else {
            WriteLE(stream, (UInt32)face.GetVertices().size());
            WriteLE(stream, (UInt32)vertOffset);
            WriteLE(stream, (UInt32)face.GetIndices().size());

            for (const UInt32& index : face.GetIndices()) {
                WriteLE(stream, (UInt32)index);
            }

            vertOffset += face.GetVertices().size();
        }
    }
    
    void Map::WriteVertex(std::vector<char>& stream, const Face& face)
    {
        if (face.GetFlags() & Face::FLAGS_NOMESH) {
            return;
        }

        for (UInt32 i = 0; i < face.GetVertices().size(); i++) {
            const Vertex& vert = face.GetVertices().at(i);

            WriteLE(stream, vert.position.x / DOWNSCALE);
            WriteLE(stream, vert.position.y / DOWNSCALE);
            WriteLE(stream, vert.position.z / DOWNSCALE);

            WriteLE(stream, vert.normal.x);
            WriteLE(stream, vert.normal.y);
            WriteLE(stream, vert.normal.z);

            WriteLE(stream, vert.texcoord.x);
            WriteLE(stream, vert.texcoord.y);
        }
    }
    
}