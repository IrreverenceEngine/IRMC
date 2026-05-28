#include <IRX_Common.hpp>
#include <IRX_Compression.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Write.hpp>

#include <vector>
#include <fstream>

namespace IRMC {

    void Map::CompileMap(const char* outpath, bool compress)
    {
        for (UInt8 i = 0; i < Stage::_COUNT; i++) {
            Stage* stage = m_Stages[i];

            if (!stage) {
                continue;
            }

            if (!stage->Run(m_StageIn, m_StageOut)) {
                IRX_MSG(ERROR, "Failed running stage: \"%s\"", Stage::NAMES[i]);
                return;
            }
        }

        std::vector<char> streams[LUMPTYPE__COUNT];
        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            streams[i].reserve(1 << 16);
        }

        std::map<std::string, UInt32> matOffsets;

        WriteMaterialTable(streams[LUMPTYPE_MATERIALTABLE], matOffsets);

        for (Entity& ent : m_Entities) {
            WriteEntity(streams[LUMPTYPE_ENTITIES], ent);

            for (Brush& brush : ent.GetBrushes()) {
                WriteBrush(streams[LUMPTYPE_BRUSHES], brush);

                for (Face& face : brush.GetFaces()) {
                    WriteFace(streams[LUMPTYPE_FACES], face, matOffsets);
                    WriteVertex(streams[LUMPTYPE_VERTICES], face);
                }
            }
        }

        WriteNavTiles(streams[LUMPTYPE_NAVTILES]);

        std::vector<char> lumps;

        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            std::vector<char>& lumpStr = streams[i];
            lumps.insert(lumps.end(), lumpStr.begin(), lumpStr.end());
        }

        UInt64 clumpsize;
        const UInt8* clumps = nullptr;
        IRX_DEFER({ if (clumps) delete[] clumps; });

        if (compress) {
            clumps = Compress((UInt8*)lumps.data(), lumps.size(), &clumpsize);
        }

        std::ofstream outStream(outpath, std::ios::binary);
        WriteLE(outStream, m_Header.magic);
        WriteLE(outStream, m_Header.version);
        WriteLE(outStream, (UInt64)(compress ? lumps.size() : 0));

        UInt32 offset = sizeof(m_Header);
        for (UInt32 i = 0; i < LUMPTYPE__COUNT; i++) {
            UInt32 streamLen = streams[i].size();
            WriteLE(outStream, offset);
            WriteLE(outStream, streamLen);

            offset += streamLen;
        }

        outStream.write(compress ? (char*)clumps : lumps.data(), compress ? clumpsize : lumps.size());
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
                            IRX_MSG(FATAL, "Material Name cannot be longer than 255");
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

        WriteLE(stream, (UInt32)brush.GetFlags());

        faceCount = brush.GetFaces().size();
        WriteLE(stream, (UInt32)faceCount);
        WriteLE(stream, (UInt32)faceOffset);

        const glm::vec3& origin = brush.GetOrigin();
        WriteLE(stream, (Float32)origin.x);
        WriteLE(stream, (Float32)origin.y);
        WriteLE(stream, (Float32)origin.z);

        const AABB& aabb = brush.GetAABB();
        WriteLE(stream, (Float32)aabb.min.x);
        WriteLE(stream, (Float32)aabb.min.y);
        WriteLE(stream, (Float32)aabb.min.z);

        WriteLE(stream, (Float32)aabb.max.x);
        WriteLE(stream, (Float32)aabb.max.y);
        WriteLE(stream, (Float32)aabb.max.z);

        WriteLE(stream, (UInt32)brush.GetConvex().size());
        for (const auto& point : brush.GetConvex()) {
            WriteLE(stream, (Float32)point.x);
            WriteLE(stream, (Float32)point.y);
            WriteLE(stream, (Float32)point.z);
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

            WriteLE(stream, vert.position.x);
            WriteLE(stream, vert.position.y);
            WriteLE(stream, vert.position.z);

            WriteLE(stream, vert.normal.x);
            WriteLE(stream, vert.normal.y);
            WriteLE(stream, vert.normal.z);

            WriteLE(stream, vert.texcoord.x);
            WriteLE(stream, vert.texcoord.y);
        }
    }

    void Map::WriteNavTiles(std::vector<char>& stream)
    {
        std::vector<char>& navdata = m_StageOut.navdata;
        Write(stream, (const char*)navdata.data(), navdata.size());
    }

}
