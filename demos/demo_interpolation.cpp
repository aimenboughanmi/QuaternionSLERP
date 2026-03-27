#include "geometry/Cube.h"
#include "math/Quat.h"
#include "raylib.h"
#include <vector>

#include <sstream>
#include <iomanip>
#include <filesystem> // C++17 for folder creation

struct DemoCube {
    const char* label;
    Vector3 offset;
    Color color;
    Quat (*interpolate)(const Quat&, const Quat&, double);
    bool useRawConjugate;
};

std::vector<std::pair<int,int>> cubeEdges() {
    return {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };
}

std::vector<Vec3> transformCube(const std::vector<Vec3>& cube,
                                 const Quat& rotation,
                                 const Vec3& offset,
                                 bool useRawConjugate) {
    std::vector<Vec3> result;
    result.reserve(cube.size());

    for(const auto& v : cube) {
        Vec3 rotated;
        if(useRawConjugate) {
            Quat qv{0.0, v.x, v.y, v.z};
            Quat conj{rotation.w, -rotation.x, -rotation.y, -rotation.z};
            Quat r = rotation * qv * conj;
            rotated = Vec3{r.x, r.y, r.z};
        } else {
            rotated = rotation.rotate(v);
        }
        result.push_back(rotated + offset);
    }

    return result;
}

void drawCube(const std::vector<Vec3>& vertices, const std::vector<std::pair<int,int>>& edges, Color color) {
    for(const auto& e : edges) {
        const Vec3& a = vertices[e.first];
        const Vec3& b = vertices[e.second];
        DrawLine3D({(float)a.x,(float)a.y,(float)a.z}, {(float)b.x,(float)b.y,(float)b.z}, color);
    }
}

int main() {
    constexpr double kPi = 3.141592653589793;

    std::vector<Vec3> cube = Cube::createUnitCube();
    std::vector<std::pair<int,int>> edges = cubeEdges();

    Vec3 axis{0.0, 0.0, 1.0};
    Quat qStart = Quat::fromAxisAngle(axis, 0.0);
    Quat qEnd   = Quat::fromAxisAngle(axis, kPi);

    std::vector<DemoCube> demos = {
        {"rawLERP", {-4.0f, 0.0f, 0.0f}, MAROON, &Quat::LERP,  true},
        {"NLERP",   { 0.0f, 0.0f, 0.0f}, GREEN,  &Quat::NLERP, false},
        {"SLERP",   { 4.0f, 0.0f, 0.0f}, BLUE,   &Quat::SLERP, false}
    };

    InitWindow(1200, 600, "Quaternion Interpolation Demo");

    // --- Frame counter and output folder ---
    int frameCounter = 0;
    std::filesystem::create_directory("frames"); // ensure folder exists


    Camera3D camera{};
    camera.position = {0.0f, 6.0f, 12.0f};
    camera.target   = {0.0f, 0.0f, 0.0f};
    camera.up       = {0.0f, 1.0f, 0.0f};
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const float duration = 4.0f;
    float elapsed = 0.0f;
    bool paused = false;

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_SPACE)) paused = !paused;
        if(!paused) elapsed += GetFrameTime();
        
        if(elapsed > duration) elapsed = 0.0f;

        /*
        // Only one loop for GIFS
        
        if(elapsed > duration) {
            elapsed = duration;
            paused = true; // stop animation
            CloseWindow();
        }
        if(elapsed < 0.0f) elapsed = 0.0f;
        */
        


        float t = elapsed / duration;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        DrawGrid(20, 1.0f);

        for(const auto& demo : demos) {
            Quat rotation = demo.interpolate(qStart, qEnd, t);
            auto vertices = transformCube(cube, rotation,
                                         Vec3{demo.offset.x, demo.offset.y, demo.offset.z},
                                         demo.useRawConjugate);

            drawCube(vertices, edges, demo.color);
            DrawSphere(demo.offset, 0.06f, demo.color);

            // Draw label above cube
            Vector3 labelPos = { demo.offset.x, demo.offset.y + 1.2f, demo.offset.z };
            Vector2 screenPos = GetWorldToScreen(labelPos, camera);
            DrawText(demo.label, (int)screenPos.x - 20, (int)screenPos.y, 20, demo.color);
        }

        EndMode3D();

        DrawText(paused ? "PAUSED [SPACE] play" : "PLAYING [SPACE] pause", 20, 20, 20, DARKGRAY);
        DrawText("Red: rawLERP | Green: NLERP | Blue: SLERP", 20, 50, 20, DARKGRAY);

        EndDrawing();

        /*
        // --- Save frame for GIF ---
        std::ostringstream filename;
        filename << "frames/frame_" << std::setfill('0') << std::setw(4) << frameCounter << ".png";
        TakeScreenshot(filename.str().c_str());
        frameCounter++;
        */
    }

    CloseWindow();
    return 0;
}