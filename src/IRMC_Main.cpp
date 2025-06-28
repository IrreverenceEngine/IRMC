#include <IRMC_Brush.hpp>
#include <IRMC_Plane.hpp>
#include <IRMC_Log.hpp>
#include <IRMC_Map.hpp>
#include <IRMC_CTypes.hpp>
#include <IRMC_Defer.hpp>

#include <cstring>
#include <raylib.h>
#include <raymath.h>

#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv)
{
    char* buffer = 0;
    IRMC_DEFER({ if (buffer) delete[] buffer; });
    IRMC::UInt64 length;
    FILE* f = fopen("bin/concept1.map", "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = new char[length + 1];
        if (buffer) {
            fread(buffer, 1, length, f);
            buffer[length] = '\0';
        }

        fclose(f);
    }

    IRMC::Map myMap(buffer);

    SetTraceLogLevel(LOG_NONE);
    InitWindow(1280, 720, "IRMC");

    DisableCursor();
    SetMousePosition(0, 0);

    Image myImage = LoadImage("bin/__TB_empty.png");
    Texture myTexture = LoadTextureFromImage(myImage);
    Material myMaterial = LoadMaterialDefault();
    myMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = myTexture;

    Camera3D cam = {
        {0, 0, 0},
        {0, 0, -1},
        {0, 1, 0},
        70.0f,
        CAMERA_PERSPECTIVE
    };

    std::vector<Mesh> meshList;

    for (IRMC::Entity& ent : myMap.m_Entities) {
        for (const IRMC::Brush& brush : ent.GetBrushes()) {
            std::vector<glm::vec3> vert = brush.GetVisibleVertices();
            std::vector<glm::vec2> texcoords = brush.GetVisibleTexcoords();
            std::vector<glm::vec3> norm = brush.GetVisibleNormals();

            Mesh mesh = { 0 };
            mesh.vertexCount = vert.size();
            mesh.triangleCount = mesh.vertexCount / 3;
            mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
            mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
            mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

            memcpy(mesh.vertices, vert.data(), mesh.vertexCount * 3 * sizeof(float));
            memcpy(mesh.texcoords, texcoords.data(), mesh.vertexCount * 2 * sizeof(float));
            memcpy(mesh.normals, norm.data(), mesh.vertexCount * 3 * sizeof(float));
            UploadMesh(&mesh, false);

            meshList.emplace_back(std::move(mesh));
        }
    }

    Matrix meshMat = MatrixScale(0.0625 / 4.0, 0.0625 / 4.0, 0.0625 / 4.0);
    while (!WindowShouldClose()) {
        UpdateCamera(&cam, CAMERA_FREE);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(cam);

        for (const Mesh& mesh : meshList) {
            DrawMesh(mesh, myMaterial, meshMat);
        }

        EndMode3D();

        DrawTexture(myTexture, 0, 0, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}