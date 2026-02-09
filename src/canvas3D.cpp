
#include <cmath>

#include "canvas.hpp"
#include "base/sugars.hpp"

#include "canvas_utils.cpp"

void Draw3DPoint(CanvasBitMap* bitmap, Point3D p) {
    u32 max_mag = (bitmap->width < bitmap->height) ? bitmap->width / 2 : bitmap->height / 2;

    i32 draw_x = (i32)((p.x / (1.0f + p.z)) * (f32)max_mag);
    i32 draw_y = (i32)((p.y / (1.0f + p.z)) * (f32)max_mag);

    DrawSquareC(bitmap, draw_x, draw_y, GOLD, 20);
}

internal void DrawLine3D(CanvasBitMap* bitmap, Point3D p1, Point3D p2) {
    u32 max_mag = (bitmap->width < bitmap->height) ? bitmap->width / 2 : bitmap->height / 2;

    i32 draw_x1 = (i32)((p1.x / (1.0f + p1.z)) * (f32)max_mag);
    i32 draw_y1 = (i32)((p1.y / (1.0f + p1.z)) * (f32)max_mag);

    i32 draw_x2 = (i32)((p2.x / (1.0f + p2.z)) * (f32)max_mag);
    i32 draw_y2 = (i32)((p2.y / (1.0f + p2.z)) * (f32)max_mag);

    BLine(bitmap, draw_x1, draw_y1, draw_x2, draw_y2, GOLD, 10);
}

Point3D Rotate3DXZ(Point3D p, f32 theta) {
    f32 cos = cosf(theta);
    f32 sin = sinf(theta);

    return Point3D{
        (p.x * cos) + (p.z * sin),
        p.y,
        (-p.x * sin) + (p.z * cos),
    };
}

internal void Render3DScene(CanvasBitMap* bitmap, CanvasInput* input, CanvasState* state) {

    // Cube as vertices and edges ----------------------- //
    Point3D points[] = {
        Point3D{0.25f,  0.25f,  0.25f },
        Point3D{-0.25f, 0.25f,  0.25f },
        Point3D{-0.25f, -0.25f, 0.25f },
        Point3D{0.25f,  -0.25f, 0.25f },

        Point3D{0.25f,  0.25f,  -0.25f},
        Point3D{-0.25f, 0.25f,  -0.25f},
        Point3D{-0.25f, -0.25f, -0.25f},
        Point3D{0.25f,  -0.25f, -0.25f},
    };

    u32 edges[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},

        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},

        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7},
    };
    // -------------------------------------------------- //

    // Drawing Vertices
    for (u32 idx = 0; idx < ArrayLen(points); idx += 1) {

        Point3D p  = points[idx];
        Point3D rp = Rotate3DXZ(p, state->theta);

        // Draw3DPoint(bitmap, rp);
    }


    // Drawing Edges
    for (u32 idx = 0; idx < 12; idx += 1) {

        Point3D p1 = points[edges[idx][0]];
        Point3D p2 = points[edges[idx][1]];

        Point3D rp1 = Rotate3DXZ(p1, state->theta);
        Point3D rp2 = Rotate3DXZ(p2, state->theta);

        rp1.x += state->dx;
        rp2.x += state->dx;

        rp1.y += state->dy;
        rp2.y += state->dy;

        rp1.z += state->dz;
        rp2.z += state->dz;

        DrawLine3D(bitmap, rp1, rp2);
    }

    // Update to state
    // state->theta += Pi32 * 0.01f;
    state->dx += (input->keyboard.D.ended_down) ? 0.01f : 0.0f;
    state->dx -= (input->keyboard.A.ended_down) ? 0.01f : 0.0f;

    state->dy += (input->keyboard.E.ended_down) ? 0.01f : 0.0f;
    state->dy -= (input->keyboard.Q.ended_down) ? 0.01f : 0.0f;

    state->dz += (input->keyboard.W.ended_down) ? 0.01f : 0.0f;
    state->dz -= (input->keyboard.S.ended_down) ? 0.01f : 0.0f;
}
