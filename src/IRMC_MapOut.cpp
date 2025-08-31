#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Defer.hpp>
#include <IRMC_QUtils.hpp>

#include <SDL3/SDL_endian.h>

#include <vector>
#include <fstream>

#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

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
        for (Entity& ent : m_Entities) {
            for (Brush& brush : ent.GetBrushes()) {
                for (Face& face : brush.GetFaces()) {
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

    void Map::WriteEntity(std::vector<char>& stream, Entity& ent)
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

    void Map::WriteBrush(std::vector<char>& stream, Brush& brush)
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

    void Map::WriteFace(std::vector<char>& stream, Face& face, std::map<std::string, UInt32>& matoffsets)
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
    
    void Map::WriteVertex(std::vector<char>& stream, Face& face)
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
        const UInt32 TILE_SIZE = 512;
        const UInt32 MAX_POLYS = 2048;

        const Float32 AGENT_HEIGHT = 112.0f;
        const Float32 AGENT_CLIMB = 16.0f;
        const Float32 AGENT_RADIUS = 24.0f;

        const Float32 CELL_SIZE = AGENT_RADIUS / 8.0f;
        const Float32 CELL_HEIGHT = AGENT_CLIMB / 4.0f;

        struct FaceInfo {
            // un-epicool copying (forgive me)
            std::vector<glm::vec3> points;
            std::vector<Int32> indices;
            AABB aabb;
        };

        std::vector<FaceInfo> worldFaces;
        for (Entity& ent : m_Entities) {
            if (ent.GetKeyValue("classname") != "worldspawn") {
                continue;
            }

            for (Brush& brush : ent.GetBrushes()) {
                worldFaces.reserve(worldFaces.size() + brush.GetFaces().size());

                for (Face& face : brush.GetFaces()) {
                    if (face.GetFlags() & Face::FLAGS_UNWALKABLE) {
                        continue;
                    }

                    FaceInfo info;
                    info.aabb = face.GetAABB();

                    info.points.reserve(face.GetVertices().size());
                    for (const Vertex& vert : face.GetVertices()) {
                        info.points.emplace_back(vert.position);
                    }

                    info.indices.reserve(face.GetIndices().size());
                    for (Int32 index : face.GetIndices()) {
                        info.indices.emplace_back(index);
                    }

                    worldFaces.emplace_back(info);
                }
            }
        }

        if (worldFaces.size() == 0) {
            return;
        }

        rcContext ctx(false);

        rcConfig cfg = { 0 };
        cfg.cs = CELL_SIZE;
        cfg.ch = CELL_HEIGHT;
        cfg.walkableSlopeAngle = 30.0f;
        cfg.walkableHeight = (UInt32)ceilf(AGENT_HEIGHT / CELL_HEIGHT);
        cfg.walkableClimb  = (UInt32)floorf(AGENT_CLIMB / CELL_HEIGHT);
        cfg.walkableRadius = (UInt32)ceilf(AGENT_RADIUS / CELL_SIZE);
        cfg.maxEdgeLen = 16;
        cfg.maxSimplificationError = 0.2f;
        cfg.maxVertsPerPoly = 6;
        cfg.borderSize = cfg.walkableRadius + 2;
        cfg.tileSize = TILE_SIZE;
        cfg.width = cfg.tileSize + cfg.borderSize * 2;
        cfg.height = cfg.tileSize + cfg.borderSize * 2;
        cfg.detailSampleDist = cfg.cs * 3.0f;
        cfg.detailSampleMaxError = cfg.ch * 2.0f;
        cfg.minRegionArea = 16 * 16;
        cfg.mergeRegionArea = 32 * 32;

        memcpy(&cfg.bmin[0], &m_NavAABB.min, sizeof(m_NavAABB.min));
        memcpy(&cfg.bmax[0], &m_NavAABB.max, sizeof(m_NavAABB.max));

        Int32 gw = (Int32)((cfg.bmax[0] - cfg.bmin[0]) / cfg.cs);
        Int32 gh = (Int32)((cfg.bmax[2] - cfg.bmin[2]) / cfg.cs);
        Int32 tw = (gw + cfg.tileSize - 1) / cfg.tileSize;
        Int32 th = (gh + cfg.tileSize - 1) / cfg.tileSize;

        dtNavMeshParams params = {};
        params.maxTiles = tw * th;
        params.maxPolys = MAX_POLYS;
        params.tileWidth = cfg.tileSize * cfg.cs;
        params.tileHeight = cfg.tileSize * cfg.cs;
        memcpy(&params.orig[0], &m_NavAABB.min, sizeof(m_NavAABB.min));

        WriteLE(stream, (Int32)params.maxTiles);
        WriteLE(stream, (Int32)params.maxPolys);
        WriteLE(stream, (Float32)params.tileWidth);
        WriteLE(stream, (Float32)params.tileHeight);
        WriteLE(stream, (Float32)params.orig[0]);
        WriteLE(stream, (Float32)params.orig[1]);
        WriteLE(stream, (Float32)params.orig[2]);

        for (Int32 ty = 0; ty < th; ty++) {
            for (Int32 tx = 0; tx < tw; tx++) {

                AABB tileAABB = {};
                Float32 border = cfg.borderSize * cfg.cs;
                tileAABB.min.x = cfg.bmin[0] + tx * cfg.tileSize * cfg.cs - border;
                tileAABB.min.z = cfg.bmin[2] + ty * cfg.tileSize * cfg.cs - border;

                tileAABB.max.x = tileAABB.min.x + (cfg.tileSize + cfg.borderSize*2) * cfg.cs;
                tileAABB.max.z = tileAABB.min.z + (cfg.tileSize + cfg.borderSize*2) * cfg.cs;

                tileAABB.min.y = cfg.bmin[1];
                tileAABB.max.y = cfg.bmax[1];

                rcHeightfield* hfield = rcAllocHeightfield();
                IRMC_DEFER({ rcFreeHeightField(hfield); });

                rcCreateHeightfield(
                    &ctx,
                    *hfield,
                    cfg.width,
                    cfg.height,
                    (Float32*)&tileAABB.min,
                    (Float32*)&tileAABB.max,
                    cfg.cs,
                    cfg.ch
                );

                for (FaceInfo& face : worldFaces) {
                    if (!face.aabb.Intersects(tileAABB) || face.indices.size() < 3) {
                        continue;
                    }

                    Int32 triNum = face.indices.size() / 3;

                    std::vector<UInt8> areas(triNum);
                    rcMarkWalkableTriangles(&ctx,
                        cfg.walkableSlopeAngle,
                        (Float32*)face.points.data(),
                        face.points.size(),
                        face.indices.data(), triNum,
                        areas.data()
                    );

                    rcRasterizeTriangles(&ctx,
                        (Float32*)face.points.data(),
                        face.points.size(),
                        face.indices.data(),
                        (const UInt8*)areas.data(),
                        triNum,
                        *hfield,
                        cfg.walkableClimb
                    );

                }

                rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *hfield);
                rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *hfield);
                rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *hfield);

                rcCompactHeightfield* chf = rcAllocCompactHeightfield();
                IRMC_DEFER({ rcFreeCompactHeightfield(chf); });
                rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *hfield, *chf);

                rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf);

                rcBuildDistanceField(&ctx, *chf);
                rcBuildRegions(&ctx, *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea);

                rcContourSet* cset = rcAllocContourSet();
                IRMC_DEFER({ rcFreeContourSet(cset); });
                rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset);

                rcPolyMesh* pmesh = rcAllocPolyMesh();
                IRMC_DEFER({ rcFreePolyMesh(pmesh); });
                rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh);

                rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
                IRMC_DEFER({ rcFreePolyMeshDetail(dmesh); });
                rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh);

                std::vector<UInt8> polyAreas(pmesh->npolys, RC_WALKABLE_AREA);
                std::vector<UInt16> polyFlags(pmesh->npolys, 1);

                dtNavMeshCreateParams tileParams = {};
                tileParams.verts = pmesh->verts;
                tileParams.vertCount = pmesh->nverts;
                tileParams.polys = pmesh->polys;
                tileParams.polyCount = pmesh->npolys;
                tileParams.nvp = pmesh->nvp;
                tileParams.detailMeshes = dmesh->meshes;
                tileParams.detailVerts = dmesh->verts;
                tileParams.detailVertsCount = dmesh->nverts;
                tileParams.detailTris = dmesh->tris;
                tileParams.detailTriCount = dmesh->ntris;
                tileParams.walkableHeight = AGENT_HEIGHT;
                tileParams.walkableRadius = AGENT_RADIUS;
                tileParams.walkableClimb  = AGENT_CLIMB;
                tileParams.cs = cfg.cs;
                tileParams.ch = cfg.ch;
                tileParams.tileX = tx;
                tileParams.tileY = ty;
                tileParams.tileLayer = 0;
                tileParams.polyAreas = polyAreas.data();
                tileParams.polyFlags = polyFlags.data();

                tileParams.bmin[0] = cfg.bmin[0] + tx * cfg.tileSize * cfg.cs;
                tileParams.bmin[1] = cfg.bmin[1];
                tileParams.bmin[2] = cfg.bmin[2] + ty * cfg.tileSize * cfg.cs;

                tileParams.bmax[0] = tileParams.bmin[0] + cfg.tileSize * cfg.cs;
                tileParams.bmax[1] = cfg.bmax[1];
                tileParams.bmax[2] = tileParams.bmin[2] + cfg.tileSize * cfg.cs;

                UInt8* data = nullptr;
                Int32 dataSize = 0;
                if (!dtCreateNavMeshData(&tileParams, &data, &dataSize)) {
                    continue;
                }

                UInt32 uDataSize = dataSize;

                WriteLE(stream, (UInt32)uDataSize);
                Write(stream, (const char*)data, uDataSize);
            }
        }
    }

}
