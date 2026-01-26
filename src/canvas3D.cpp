
#include <cmath>

#include "canvas.hpp"
#include "canvas_sugars.hpp"

#include "canvas_utils.cpp"

internal void Draw3DPoint(CanvasBitMap* bitmap, Point3D p) {
    uint32 max_mag = (bitmap->Width < bitmap->Height) ? bitmap->Width / 2 : bitmap->Height / 2;

    int32 draw_x = (int32)(0.8f * (p.x / p.z) * (float32)max_mag);
    int32 draw_y = (int32)(0.8f * (p.y / p.z) * (float32)max_mag);

    DrawSquareC(bitmap, draw_x, draw_y, GOLD, 20);
}

internal void DrawLine3D(CanvasBitMap* bitmap, Point3D p1, Point3D p2) {
    uint32 max_mag = (bitmap->Width < bitmap->Height) ? bitmap->Width / 2 : bitmap->Height / 2;

    int32 draw_x1 = (int32)(0.8f * (p1.x / p1.z) * (float32)max_mag);
    int32 draw_y1 = (int32)(0.8f * (p1.y / p1.z) * (float32)max_mag);

    int32 draw_x2 = (int32)(0.8f * (p2.x / p2.z) * (float32)max_mag);
    int32 draw_y2 = (int32)(0.8f * (p2.y / p2.z) * (float32)max_mag);

    BLine(bitmap, draw_x1, draw_y1, draw_x2, draw_y2, GOLD, 10);
}

internal Point3D Rotate3DXZ(Point3D p, float32 theta) {
    float32 cos = cosf(theta);
    float32 sin = sinf(theta);

    return Point3D{
        (p.x * cos) - (p.z * sin),
        p.y,
        (p.x * sin) + (p.z * cos),
    };
}

internal void Render3DScene(CanvasBitMap* bitmap, CanvasInput* input, CanvasState* state) {

    Point3D points[] = { Point3D{0.25f,  0.25f,  0.25f },
        Point3D{-0.25f, 0.25f,  0.25f },
        Point3D{-0.25f, -0.25f, 0.25f },
        Point3D{0.25f,  -0.25f, 0.25f },

        Point3D{0.25f,  0.25f,  -0.25f},
        Point3D{-0.25f, 0.25f,  -0.25f},
        Point3D{-0.25f, -0.25f, -0.25f},
        Point3D{0.25f,  -0.25f, -0.25f},
    };

    uint32 edges[2][4] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
    };

    uint32 edges2[4][2] = {
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7},
    };

    for (uint32 idx = 0; idx < ArrayLen(points); idx += 1) {
        points[idx].z -= state->dz;
        Draw3DPoint(bitmap, Rotate3DXZ(points[idx], state->theta));
    }

    for (uint32 idx = 0; idx < 2; idx += 1) {
        for (uint32 jdx = 0; jdx < 4; jdx += 1) {
            DrawLine3D(bitmap,
                       Rotate3DXZ(points[edges[idx][jdx]], state->theta),
                       Rotate3DXZ(points[edges[idx][(jdx + 1) % 4]], state->theta));
        }
    }

    for (uint32 idx = 0; idx < 4; idx += 1) {
        DrawLine3D(bitmap,
                   Rotate3DXZ(points[edges2[idx][0]], state->theta),
                   Rotate3DXZ(points[edges2[idx][1]], state->theta));
    }

    state->theta += Pi32 * 0.01f;
    state->dz += 0.001f;
}
