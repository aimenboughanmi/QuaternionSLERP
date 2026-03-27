#include "math/Vec3.h"
#include "math/Quat.h"
#include "geometry/Cube.h"

#include "raylib.h"
#include <vector>
#include <utility>
#include <sstream>
#include <iomanip>
#include <filesystem> // C++17 for folder creation
#include <cmath>

int main() {

    // --- Cube setup ---
    std::vector<Vec3> cube = Cube::createUnitCube();
    std::vector<std::pair<int,int>> edges = {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    // --- Animation parameters ---
    //Vec3 axis{0.0f, 0.0f, 1.0f};
    Vec3 axis{1.0f, 0.0f, 0.0f};
    //Vec3 axis{0.0f, 1.0f, 0.0f};
    
    const int FPS = 60; // same as SetTargetFPS
    float duration = 5.0f;
    float elapsed  = 0.0f;
    bool paused = false;

    // --- Track multiple vertices ---
    //std::vector<int> trackedVertices = {0,1,3,2}; // front face
    //std::vector<int> trackedVertices = {0,1,2,3,4,5,6,7}; // front face
    //std::vector<int> trackedVertices = {1, 3, 4, 6}; // perpendicular-to-camera plane
    std::vector<int> trackedVertices = {0 , 7};
    
    const size_t maxFrames = static_cast<size_t>(duration * FPS);
    std::vector<std::vector<Vec3>> frameTrails;

    // --- Frame counter and output folder ---
    int frameCounter = 0;
    std::filesystem::create_directory("frames"); // ensure folder exists

    // --- Raylib window + camera ---
    InitWindow(800,600,"Quaternion Cube SLERP Tube");

    Camera3D camera;
    camera.position = {4.0f,4.0f,6.0f};
    camera.target   = {0.0f,0.0f,0.0f};
    camera.up       = {0.0f,1.0f,0.0f};
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    bool orbit = false;
    float camAngle = 0.0f;
    bool showQuatDebug = true;

    SetTargetFPS(FPS);

    while(!WindowShouldClose()) {

        // --- Input ---
        if(IsKeyPressed(KEY_X)){
            axis = Vec3{1,0,0};
            frameTrails.clear();
        }

        if(IsKeyPressed(KEY_Y)){
            axis = Vec3{0,1,0};
            frameTrails.clear();
        }

        if(IsKeyPressed(KEY_Z)){
            axis = Vec3{0,0,1};
            frameTrails.clear();
        }

        if(IsKeyPressed(KEY_SPACE)) paused = !paused;
        if(IsKeyPressed(KEY_O)) orbit = !orbit;
        if(IsKeyPressed(KEY_T)) showQuatDebug = !showQuatDebug;

        if(IsKeyDown(KEY_RIGHT)) elapsed += GetFrameTime() * 0.5f;
        if(IsKeyDown(KEY_LEFT))  elapsed -= GetFrameTime() * 0.5f;
        if(!paused) elapsed += GetFrameTime();

        if(elapsed > duration) elapsed = 0.0f;
        if(elapsed < 0.0f) elapsed = duration;

        // Only one loop for GIFS
        /*
        if(elapsed > duration) {
            elapsed = duration;
            paused = true; // stop animation
            CloseWindow();
        }
        if(elapsed < 0.0f) elapsed = 0.0f;
        */
        

        // Camera orbit
        if(orbit){
            camAngle += 0.01f;
            camera.position.x = 6.0f * sin(camAngle);
            camera.position.z = 6.0f * cos(camAngle);
        }

        // --- Quaternion rotation ---
        //float t = elapsed / duration;
        //Quat qStart = Quat::fromAxisAngle(axis,0.0f);
        //Quat qEnd   = Quat::fromAxisAngle(axis, 3.14159f);
        //Quat qCurrent = Quat::SLERP(qStart,qEnd,t);

        // Handles so that the cube rotates 360 degrees 
        /*
        float totalRotation = 2.0f * 3.14159f; // 360 degrees
        float halfRotation = totalRotation / 2.0f;
        float t = elapsed / duration; // 0 → 1
        Quat qCurrent;
        if(t <= 0.5f) {
            // first half: 0 → 180°
            float localT = t / 0.5f; // remap 0..0.5 → 0..1
            Quat qStart = Quat::fromAxisAngle(axis, 0.0f);
            Quat qMid   = Quat::fromAxisAngle(axis, halfRotation);
            qCurrent = Quat::SLERP(qStart, qMid, localT);
        } else {
            // second half: 180° → 360°
            float localT = (t - 0.5f) / 0.5f; // remap 0.5..1 → 0..1
            Quat qMid   = Quat::fromAxisAngle(axis, halfRotation);
            Quat qEnd   = Quat::fromAxisAngle(axis, totalRotation);
            qCurrent = Quat::SLERP(qMid, qEnd, localT);
        }
        */

        Quat qStart = Quat::fromAxisAngle({1,0,0}, 0.7f) * Quat::fromAxisAngle({0,1,0}, 0.2f);
        Quat qEnd   = Quat::fromAxisAngle({0,1,0}, 1.4f) * Quat::fromAxisAngle({0,0,1}, -0.8f);

        float t = elapsed / duration;
        Quat qCurrent = Quat::SLERP(qStart, qEnd, t);

        Vec3 ex = qCurrent.rotate({1,0,0});
        Vec3 ey = qCurrent.rotate({0,1,0});
        Vec3 ez = qCurrent.rotate({0,0,1});

        // Rotate cube vertices
        std::vector<Vec3> rotated;
        rotated.reserve(cube.size());
        for(const auto& v : cube)
            rotated.push_back(qCurrent.rotate(v));

        // Store tracked vertices
        std::vector<Vec3> currentFrame;
        for(int idx : trackedVertices) currentFrame.push_back(rotated[idx]);
        frameTrails.push_back(currentFrame);
        if(frameTrails.size() > maxFrames) frameTrails.erase(frameTrails.begin());

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        // Draw cube edges
        for(const auto& e : edges){
            Vec3 a = rotated[e.first];
            Vec3 b = rotated[e.second];
            DrawLine3D({(float)a.x,(float)a.y,(float)a.z},
                       {(float)b.x,(float)b.y,(float)b.z},
                       BLACK);
        }

        // Draw trails
        for(size_t f=1; f<frameTrails.size(); ++f){
            auto& prevFrame = frameTrails[f-1];
            auto& curFrame  = frameTrails[f];

            for(size_t v=0; v<curFrame.size(); ++v){
                Vector3 a = {(float)prevFrame[v].x,(float)prevFrame[v].y,(float)prevFrame[v].z};
                Vector3 b = {(float)curFrame[v].x,(float)curFrame[v].y,(float)curFrame[v].z};
                //DrawLine3D(a,b,PURPLE);
                DrawSphere(b, 0.03f, PURPLE);

                Vector3 c = {(float)curFrame[(v+1)%curFrame.size()].x,
                             (float)curFrame[(v+1)%curFrame.size()].y,
                             (float)curFrame[(v+1)%curFrame.size()].z};
                //DrawLine3D(b,c,PURPLE);
                DrawSphere(b, 0.03f, PURPLE);
            }
        }

        // Reference grid
        DrawGrid(10,1.0f);

        const float basisLen = 1.6f;
        DrawLine3D({0,0,0}, {(float)(ex.x*basisLen),(float)(ex.y*basisLen),(float)(ex.z*basisLen)}, MAROON);
        DrawLine3D({0,0,0}, {(float)(ey.x*basisLen),(float)(ey.y*basisLen),(float)(ey.z*basisLen)}, DARKGREEN);
        DrawLine3D({0,0,0}, {(float)(ez.x*basisLen),(float)(ez.y*basisLen),(float)(ez.z*basisLen)}, DARKBLUE);


        // Rotation axis
        Vec3 axisN = axis.normalized();
        DrawCylinderEx({(float)(-axisN.x),(float)(-axisN.y),(float)(-axisN.z)},
                       {(float)( axisN.x),(float)( axisN.y),(float)( axisN.z)},
                       0.03f,0.03f,8,ORANGE);
        DrawSphere({(float)(axisN.x),(float)(axisN.y),(float)(axisN.z)},0.08f,ORANGE);

        // Coordinate axes
        DrawLine3D({0,0,0},{2,0,0},RED);
        DrawLine3D({0,0,0},{0,2,0},GREEN);
        DrawLine3D({0,0,0},{0,0,2},BLUE);

        EndMode3D();

        // HUD + labels
        DrawText(paused ? "PAUSED  [SPACE] play  [<-][->] scrub"
                        : "PLAYING [SPACE] pause",10,10,18,DARKGRAY);
        DrawText("[X][Y][Z] change axis  [O] toggle orbit",10,32,18,DARKGRAY);

        if(showQuatDebug){
            std::string line1 = TextFormat("t: %.3f", t);
            std::string line2 = TextFormat("qStart   = (%.3f, %.3f, %.3f, %.3f)", qStart.w, qStart.x, qStart.y, qStart.z);
            std::string line3 = TextFormat("qEnd     = (%.3f, %.3f, %.3f, %.3f)", qEnd.w, qEnd.x, qEnd.y, qEnd.z);
            std::string line4 = TextFormat("qCurrent = (%.3f, %.3f, %.3f, %.3f)", qCurrent.w, qCurrent.x, qCurrent.y, qCurrent.z);

            DrawText(line1.c_str(), 10, 58, 16, GRAY);
            DrawText(line2.c_str(), 10, 78, 16, GRAY);
            DrawText(line3.c_str(), 10, 98, 16, GRAY);
            DrawText(line4.c_str(), 10,118, 16, GRAY);
        }

        Vector2 labelX = GetWorldToScreen({2,0,0}, camera);
        Vector2 labelY = GetWorldToScreen({0,2,0}, camera);
        Vector2 labelZ = GetWorldToScreen({0,0,2}, camera);

        DrawText("X", (int)labelX.x, (int)labelX.y, 20, RED);
        DrawText("Y", (int)labelY.x, (int)labelY.y, 20, GREEN);
        DrawText("Z", (int)labelZ.x, (int)labelZ.y, 20, BLUE);

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
}