#include "math/Vec3.h"
#include "math/Quat.h"
#include "geometry/Cube.h"

#include "raylib.h"
#include <vector>
#include <utility>
#include <cmath>
#include <filesystem> // C++17 for folder creation
#include <sstream>
#include <iomanip>

int main() {
    // --- Cube setup ---
    std::vector<Vec3> cube = Cube::createUnitCube();
    std::vector<std::pair<int,int>> edges = {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    // --- Animation ---
    Vec3 axis{1.0f, 0.0f, 0.0f};
    constexpr int FPS = 60;
    float duration = 5.0f;
    float elapsed = 0.0f;
    bool paused = false;

    enum class RotationMode { Axis360, PoseToPose };
    RotationMode mode = RotationMode::PoseToPose;

    // --- Trail setup ---
    std::vector<int> trackedVertices = {0, 7};
    const auto maxFrames = static_cast<size_t>(duration * FPS);
    std::vector<std::vector<Vec3>> frameTrails;

    // --- Frame counter and output folder ---
    int frameCounter = 0;
    std::filesystem::create_directory("frames"); // ensure folder exists


    // --- Window + camera ---
    InitWindow(800, 600, "Quaternion Cube SLERP Tube");
    Camera3D camera{};
    camera.position = {4.0f, 4.0f, 6.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    bool orbit = false;
    float camAngle = 0.0f;
    bool showQuatDebug = true;

    SetTargetFPS(FPS);

    while (!WindowShouldClose()) {
        // --- Input ---
        if (IsKeyPressed(KEY_X)) { axis = Vec3{1,0,0}; frameTrails.clear(); }
        if (IsKeyPressed(KEY_Y)) { axis = Vec3{0,1,0}; frameTrails.clear(); }
        if (IsKeyPressed(KEY_Z)) { axis = Vec3{0,0,1}; frameTrails.clear(); }

        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyPressed(KEY_O)) orbit = !orbit;
        if (IsKeyPressed(KEY_T)) showQuatDebug = !showQuatDebug;

        // Toggle between demos
        if (IsKeyPressed(KEY_M)) {
            mode = (mode == RotationMode::Axis360) ? RotationMode::PoseToPose : RotationMode::Axis360;
            elapsed = 0.0f;
            frameTrails.clear();
        }

        if (IsKeyDown(KEY_RIGHT)) elapsed += GetFrameTime() * 0.5f;
        if (IsKeyDown(KEY_LEFT))  elapsed -= GetFrameTime() * 0.5f;
        if (!paused) elapsed += GetFrameTime();

        if (elapsed > duration) elapsed = 0.0f;
        if (elapsed < 0.0f) elapsed = duration;

        /*
         // Only one loop for GIFS
        if(elapsed > duration) {
            elapsed = duration;
            paused = true; // stop animation
            CloseWindow();
        }
        if(elapsed < 0.0f) elapsed = 0.0f;
        */

        if (orbit) {
            camAngle += 0.01f;
            camera.position.x = 6.0f * sinf(camAngle);
            camera.position.z = 6.0f * cosf(camAngle);
        }

        // --- Quaternion rotation ---
        const float t = elapsed / duration;
        Quat qStart{};
        Quat qEnd{};
        Quat qCurrent{};

        if (mode == RotationMode::Axis360) {
            const float totalRotation = 2.0f * 3.14159f;
            const float halfRotation = totalRotation * 0.5f;

            qStart = Quat::fromAxisAngle(axis, 0.0f);
            qEnd   = Quat::fromAxisAngle(axis, totalRotation);

            if (t <= 0.5f) {
                float localT = t / 0.5f;
                Quat qMid = Quat::fromAxisAngle(axis, halfRotation);
                qCurrent = Quat::SLERP(qStart, qMid, localT);
            } else {
                float localT = (t - 0.5f) / 0.5f;
                Quat qMid = Quat::fromAxisAngle(axis, halfRotation);
                qCurrent = Quat::SLERP(qMid, qEnd, localT);
            }
        } else {

            //qStart = Quat::fromAxisAngle({1,0,0}, 0.7f) * Quat::fromAxisAngle({0,1,0}, 0.2f);
            //qEnd   = Quat::fromAxisAngle({0,1,0}, 1.4f) * Quat::fromAxisAngle({0,0,1}, -0.8f);
            //qCurrent = Quat::SLERP(qStart, qEnd, t);
            
            // Easier-to-read pose interpolation:
            // start = no rotation
            // end   = yaw +90°, then pitch +45°
            constexpr float deg2rad = 3.14159f / 180.0f;

            qStart = Quat::fromAxisAngle({0,1,0}, 0.0f); // identity
            qEnd   = Quat::fromAxisAngle({0,1,0}, 90.0f * deg2rad) *
                     Quat::fromAxisAngle({1,0,0}, 45.0f * deg2rad);

            qCurrent = Quat::SLERP(qStart, qEnd, t);
        }

        Vec3 ex = qCurrent.rotate({1,0,0});
        Vec3 ey = qCurrent.rotate({0,1,0});
        Vec3 ez = qCurrent.rotate({0,0,1});

        std::vector<Vec3> rotated;
        rotated.reserve(cube.size());
        for (const auto& v : cube) rotated.push_back(qCurrent.rotate(v));

        // --- Trail update ---
        if (!paused) {
            std::vector<Vec3> currentFrame;
            currentFrame.reserve(trackedVertices.size());
            for (int idx : trackedVertices) currentFrame.push_back(rotated[idx]);
            frameTrails.push_back(currentFrame);
            if (frameTrails.size() > maxFrames) frameTrails.erase(frameTrails.begin());
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        for (const auto& e : edges) {
            Vec3 a = rotated[e.first];
            Vec3 b = rotated[e.second];
            DrawLine3D({static_cast<float>(a.x),static_cast<float>(a.y),static_cast<float>(a.z)},
                       {static_cast<float>(b.x),static_cast<float>(b.y),static_cast<float>(b.z)},
                       BLACK);
        }

        for (size_t f = 1; f < frameTrails.size(); ++f) {
            auto& curFrame = frameTrails[f];
            for (auto & v : curFrame) {
                Vector3 p = {static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z)};
                DrawSphere(p, 0.03f, PURPLE);
            }
        }

        DrawGrid(10, 1.0f);

        constexpr float basisLen = 1.6f;
        DrawLine3D({0,0,0}, {static_cast<float>(ex.x * basisLen),static_cast<float>(ex.y * basisLen),static_cast<float>(ex.z * basisLen)}, MAROON);
        DrawLine3D({0,0,0}, {static_cast<float>(ey.x * basisLen),static_cast<float>(ey.y * basisLen),static_cast<float>(ey.z * basisLen)}, DARKGREEN);
        DrawLine3D({0,0,0}, {static_cast<float>(ez.x * basisLen),static_cast<float>(ez.y * basisLen),static_cast<float>(ez.z * basisLen)}, DARKBLUE);

        // Axis gizmo only for Axis360 mode
        if (mode == RotationMode::Axis360) {
            Vec3 axisN = axis.normalized();
            DrawCylinderEx({static_cast<float>(-axisN.x),static_cast<float>(-axisN.y),static_cast<float>(-axisN.z)},
                           {static_cast<float>(axisN.x),static_cast<float>(axisN.y),static_cast<float>(axisN.z)},
                           0.03f, 0.03f, 8, ORANGE);
            DrawSphere({static_cast<float>(axisN.x),static_cast<float>(axisN.y),static_cast<float>(axisN.z)}, 0.08f, ORANGE);
        }

        DrawLine3D({0,0,0},{2,0,0},RED);
        DrawLine3D({0,0,0},{0,2,0},GREEN);
        DrawLine3D({0,0,0},{0,0,2},BLUE);

        EndMode3D();

        DrawText(paused ? "PAUSED  [SPACE] play  [<-][->] scrub"
                        : "PLAYING [SPACE] pause", 10, 10, 18, DARKGRAY);
        DrawText("[M] mode  [X][Y][Z] axis  [O] orbit  [T] debug", 10, 32, 18, DARKGRAY);
        DrawText(mode == RotationMode::Axis360 ? "Mode: Axis 360" : "Mode: Pose-to-pose SLERP", 10, 54, 18, DARKGRAY);

        if (showQuatDebug) {
            DrawText(TextFormat("t: %.3f", t), 10, 78, 16, GRAY);
            DrawText(TextFormat("qStart   = (%.3f, %.3f, %.3f, %.3f)", qStart.w, qStart.x, qStart.y, qStart.z), 10, 98, 16, GRAY);
            DrawText(TextFormat("qEnd     = (%.3f, %.3f, %.3f, %.3f)", qEnd.w, qEnd.x, qEnd.y, qEnd.z), 10,118, 16, GRAY);
            DrawText(TextFormat("qCurrent = (%.3f, %.3f, %.3f, %.3f)", qCurrent.w, qCurrent.x, qCurrent.y, qCurrent.z), 10,138, 16, GRAY);
        }

        Vector2 labelX = GetWorldToScreen({2,0,0}, camera);
        Vector2 labelY = GetWorldToScreen({0,2,0}, camera);
        Vector2 labelZ = GetWorldToScreen({0,0,2}, camera);
        DrawText("X", static_cast<int>(labelX.x), static_cast<int>(labelX.y), 20, RED);
        DrawText("Y", static_cast<int>(labelY.x), static_cast<int>(labelY.y), 20, GREEN);
        DrawText("Z", static_cast<int>(labelZ.x), static_cast<int>(labelZ.y), 20, BLUE);

        EndDrawing();

        
        // --- Save frame for GIF ---
        /*
        std::ostringstream filename;
        filename << "frames/frame_" << std::setfill('0') << std::setw(4) << frameCounter << ".png";
        TakeScreenshot(filename.str().c_str());
        frameCounter++;
        */
        
    }

    CloseWindow();
    return 0;
}