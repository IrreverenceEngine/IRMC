#include <IRMC_Brush.hpp>
#include <IRMC_Plane.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_CTypes.hpp>

#include <cstring>
#include <raylib.h>
#include <raymath.h>

#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv)
{
    std::vector<IRMC::Brushside> brushsides {
        {
            IRMC::Plane::MakeFromPoints({ -16, 16, 0 }, { 0, 16, 16 }, { 0, 0, 0 }),
            { -0.707106, 0.707106, 0, 0 },
            { 0.408248, 0.408248, 0.816496, 0 },
            { 1, 1 }
        },
        {
            IRMC::Plane::MakeFromPoints({ 80, 16, -16 }, { 80, 16, -17 }, { 81, 16, -16 }),
            { 1, 0, 0, 0 },
            { 0, 0, 1, 0 },
            { 1, 1 }
        },
        {
            IRMC::Plane::MakeFromPoints({ 80, 16, 0 }, { 81, 16, 0 }, { 80, 17, 0 }),
            { -1, 0, 0, 0 },
            { 0, -1, 0, 0 },
            { 1, 1 }
        },
        {
            IRMC::Plane::MakeFromPoints({ 0, 16, -16 }, { 0, 17, -16 }, { 0, 1, -17 }),
            { -2.220446049250313e-16, 0, 1, 0 },
            { 0, -1, 0, 0 },
            { 1, 1 }
        }
    };

    IRMC::Map myMap(R"text(
        // Game: Generic
        // Format: Valve
        // entity 0
        {
        "mapversion" "220"
        "classname" "worldspawn"
        // brush 0
        {
        ( -24 0 0 ) ( -16 -8 16 ) ( -16 -16 8 ) __TB_empty [ -2.7755575615628914e-16 -1 -9.860761315262648e-32 20.26667 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( -16 -16 0 ) ( -24 0 0 ) ( -16 -16 8 ) __TB_empty [ -2.7755575615628914e-16 -1 -9.860761315262648e-32 0 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( -24 0 0 ) ( -16 0 16 ) ( -16 -8 16 ) __TB_empty [ -2.7755575615628914e-16 -1 -9.860761315262648e-32 10.666664 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( -16 -8 16 ) ( 0 -24 16 ) ( -16 -16 8 ) __TB_empty [ -1 2.220446049250313e-16 -2.7755575615628914e-16 0 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( -16 -16 8 ) ( 0 -24 16 ) ( -16 -16 0 ) __TB_empty [ -1 2.220446049250313e-16 -2.7755575615628914e-16 0 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( 0 -24 16 ) ( 0 -16 0 ) ( -16 -16 0 ) __TB_empty [ -1 2.220446049250313e-16 -2.7755575615628914e-16 0 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( 0 -16 0 ) ( 0 0 0 ) ( -24 0 0 ) __TB_empty [ -1 2.220446049250313e-16 -2.7755575615628914e-16 0 ] [ 2.2204460492503126e-16 1 2.220446049250313e-16 0 ] 0 0.25 0.25
        ( -16 0 16 ) ( 0 0 16 ) ( 0 -24 16 ) __TB_empty [ -1 2.220446049250313e-16 -2.7755575615628914e-16 0 ] [ -2.220446049250313e-16 -1 2.220446049250313e-16 0 ] 0 0.25 0.25
        ( 0 0 0 ) ( 0 0 16 ) ( -16 0 16 ) __TB_empty [ -1 2.220446049250313e-16 -2.7755575615628914e-16 0 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        ( 0 -24 16 ) ( 0 0 16 ) ( 0 0 0 ) __TB_empty [ 0 1 2.465190328815662e-32 0 ] [ -2.7755575615628914e-16 0 1 0 ] 0 0.25 0.25
        }
        }
    )text");

    const IRMC::Brush& myBrush = myMap.m_Entities.at(0).GetBrushes().at(0);

    std::vector<glm::vec3> vert = myBrush.GetTotalVertices();
    std::vector<glm::vec2> texcoords = myBrush.GetTotalTexcoords();
    std::vector<glm::vec3> norm = myBrush.GetTotalNormals();

    SetTraceLogLevel(LOG_NONE);
    InitWindow(1280, 720, "IRMC");

    DisableCursor();

    Image myImage = LoadImage("bin/__TB_empty.png");
    Texture myTexture = LoadTextureFromImage(myImage);
    Material myMaterial = LoadMaterialDefault();
    myMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = myTexture;

    Mesh mesh = { 0 };
    mesh.vertexCount = vert.size();
    mesh.triangleCount = mesh.vertexCount / 3;
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)MemAlloc(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));

    memcpy(mesh.vertices, vert.data(), mesh.vertexCount*3*sizeof(float));
    memcpy(mesh.texcoords, texcoords.data(), mesh.vertexCount*2*sizeof(float));
    memcpy(mesh.normals, norm.data(), mesh.vertexCount*3*sizeof(float));
    UploadMesh(&mesh, false);

    Camera3D cam = {
        {0, 1, -2},
        {0, 0, 0},
        {0, 1, 0},
        70.0f,
        CAMERA_PERSPECTIVE
    };

    while (!WindowShouldClose()) {
        UpdateCamera(&cam, CAMERA_FREE);
        UpdateCamera(&cam, CAMERA_FREE);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(cam);

        DrawMesh(mesh, myMaterial, MatrixIdentity());

        DrawLine3D({0, 0, 0}, { 99999999.0f, 0, 0 }, RED);
        DrawLine3D({0, 0, 0}, { 0, 99999999.0f, 0 }, GREEN);
        DrawLine3D({0, 0, 0}, { 0, 0, -99999999.0f }, BLUE);
        // myBrush.DebugDraw();

        EndMode3D();

        DrawTexture(myTexture, 0, 0, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}