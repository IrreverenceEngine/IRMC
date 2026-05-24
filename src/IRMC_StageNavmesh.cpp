#include <IRMC_StageNavmesh.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_Brush.hpp>
#include <IRMC_Write.hpp>

#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

namespace IRMC {
    bool StageNavmesh::Run(const MapStageInput& in, MapStageOutput& out)
    {
        std::vector<char>& navdata = out.navdata;

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
        for (const Entity& ent : in.ents) {
            if (ent.GetKeyValue("classname") != "worldspawn") {
                continue;
            }

            for (const Brush& brush : ent.GetBrushes()) {
                worldFaces.reserve(worldFaces.size() + brush.GetFaces().size());

                for (const Face& face : brush.GetFaces()) {
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
            return false;
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

        memcpy(&cfg.bmin[0], &in.navbounds.min, sizeof(in.navbounds.min));
        memcpy(&cfg.bmax[0], &in.navbounds.max, sizeof(in.navbounds.max));

        Int32 gw = (Int32)((cfg.bmax[0] - cfg.bmin[0]) / cfg.cs);
        Int32 gh = (Int32)((cfg.bmax[2] - cfg.bmin[2]) / cfg.cs);
        Int32 tw = (gw + cfg.tileSize - 1) / cfg.tileSize;
        Int32 th = (gh + cfg.tileSize - 1) / cfg.tileSize;

        dtNavMeshParams params = {};
        params.maxTiles = tw * th;
        params.maxPolys = MAX_POLYS;
        params.tileWidth = cfg.tileSize * cfg.cs;
        params.tileHeight = cfg.tileSize * cfg.cs;
        memcpy(&params.orig[0], &in.navbounds.min, sizeof(in.navbounds.min));

        WriteLE(navdata, (Int32)params.maxTiles);
        WriteLE(navdata, (Int32)params.maxPolys);
        WriteLE(navdata, (Float32)params.tileWidth);
        WriteLE(navdata, (Float32)params.tileHeight);
        WriteLE(navdata, (Float32)params.orig[0]);
        WriteLE(navdata, (Float32)params.orig[1]);
        WriteLE(navdata, (Float32)params.orig[2]);

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
                IRX_DEFER({ rcFreeHeightField(hfield); });

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
                IRX_DEFER({ rcFreeCompactHeightfield(chf); });
                rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *hfield, *chf);

                rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf);

                rcBuildDistanceField(&ctx, *chf);
                rcBuildRegions(&ctx, *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea);

                rcContourSet* cset = rcAllocContourSet();
                IRX_DEFER({ rcFreeContourSet(cset); });
                rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset);

                rcPolyMesh* pmesh = rcAllocPolyMesh();
                IRX_DEFER({ rcFreePolyMesh(pmesh); });
                rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh);

                rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
                IRX_DEFER({ rcFreePolyMeshDetail(dmesh); });
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

                navdata.reserve(navdata.size() + uDataSize + sizeof(UInt32));

                WriteLE(navdata, uDataSize);
                Write(navdata, (const char*)data, uDataSize);
            }
        }

        return true;
    };
}
