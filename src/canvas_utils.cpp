#ifndef CANVAS_UTILS_H
#define CANVAS_UTILS_H
// ---------------------------------------------------------------------------------------------- //

#include "canvas.hpp"

typedef struct {
    float32 x;
    float32 y;
    float32 z;
} Point3D;

typedef struct {
    float32 x;
    float32 y;
} Point;


// Colors --------------------------------------------------------------------------------------- //
typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 pad;
} Color;

#define BLACK Color{0, 0, 0, 0}
#define WHITE Color{255, 255, 255, 0}

#define RED   Color{255, 0, 0, 0}
#define GREEN Color{0, 255, 0, 0}
#define BLUE  Color{0, 0, 255, 0}

#define YELLOW  Color{255, 255, 0, 0}
#define CYAN    Color{0, 255, 255, 0}
#define MAGENTA Color{255, 0, 255, 0}

#define GRAY       Color{128, 128, 128, 0}
#define DARK_GRAY  Color{64, 64, 64, 0}
#define LIGHT_GRAY Color{192, 192, 192, 0}
#define ORANGE     Color{255, 165, 0, 0}

#define BROWN  Color{165, 42, 42, 0}
#define PINK   Color{255, 192, 203, 0}
#define PURPLE Color{128, 0, 128, 0}
#define TEAL   Color{0, 128, 128, 0}
#define NAVY   Color{0, 0, 128, 0}

#define OLIVE  Color{128, 128, 0, 0}
#define MAROON Color{128, 0, 0, 0}
#define LIME   Color{50, 205, 50, 0}
#define GOLD   Color{255, 215, 0, 0}
#define SILVER Color{192, 192, 192, 0}
// Colors --------------------------------------------------------------------------------------- //

inline internal void DrawPixel(uint32* draw_pixel, uint8 red, uint8 green, uint8 blue, uint8 pad) {
    *draw_pixel = ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
}

inline internal void
DrawPixel(CanvasBitMap* bitmap, int32 x, int32 y, uint8 red, uint8 blue, uint8 green, uint8 pad) {

    uint32 dx = x + (bitmap->Width / 2);
    uint32 dy = -y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        uint8*  draw_row   = ((uint8*)bitmap->Memory) + (bitmap->Pitch * dy);
        uint32* draw_pixel = ((uint32*)draw_row + dx);

        *draw_pixel =
            ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
    }
}

internal void DrawPixel(CanvasBitMap* bitmap, int32 x, int32 y, Color c) {

    uint32 dx = x + (bitmap->Width / 2);
    uint32 dy = -y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        uint8*  draw_row   = ((uint8*)bitmap->Memory) + (bitmap->Pitch * dy);
        uint32* draw_pixel = ((uint32*)draw_row + dx);

        *draw_pixel = ((uint32)c.pad << 24) | ((uint32)c.red << 16) | ((uint32)c.green << 8) |
                      ((uint32)c.blue);
    }
}

internal void
DrawRectangle(CanvasBitMap* bitmap, int32 x, int32 y, Color c, uint32 width, uint32 height) {

    for (uint32 draw_y = 0; draw_y < height; draw_y += 1) {
        for (uint32 draw_x = 0; draw_x < width; draw_x += 1) {
            DrawPixel(bitmap, draw_x + x, draw_y + y, c);
        }
    }
}

inline internal void DrawSquare(CanvasBitMap* bitmap, int32 x, int32 y, Color c, uint32 size) {
    DrawRectangle(bitmap, x, y, c, size, size);
}

inline internal void DrawSquareC(CanvasBitMap* BitMap, int32 X, int32 Y, Color c, uint32 size) {
    DrawSquare(BitMap, X - (size / 2), Y - (size / 2), c, size);
}

void CaseyCircle(CanvasBitMap* BitMap, int32 Cx, int32 Cy, uint32 r, Color c) {
    int r2 = r + r;

    int X  = r;
    int Y  = 0;
    int dY = -2;
    int dX = r2 + r2 - 4;
    int D  = r2 - 1;

    while (Y <= X) {

        DrawPixel(BitMap, Cx - X, Cy - Y, c);
        DrawPixel(BitMap, Cx + X, Cy - Y, c);
        DrawPixel(BitMap, Cx - X, Cy + Y, c);
        DrawPixel(BitMap, Cx + X, Cy + Y, c);
        DrawPixel(BitMap, Cx - Y, Cy - X, c);
        DrawPixel(BitMap, Cx + Y, Cy - X, c);
        DrawPixel(BitMap, Cx - Y, Cy + X, c);
        DrawPixel(BitMap, Cx + Y, Cy + X, c);

        D += dY;
        dY -= 4;
        ++Y;

        if (D < 0) {
            D += dX;
            dX -= 4;
            --X;
        }
    }
}

internal void FillLine(CanvasBitMap* bitmap, int32 Cx, int32 Cy, int32 x, int32 y, Color c) {
    for (int32 i = y; i >= -y; i -= 1) {
        DrawPixel(bitmap, Cx - x, Cy - i, c);
    }
}

internal void CaseyCircleFill(CanvasBitMap* BitMap, int32 Cx, int32 Cy, uint32 r, Color c) {
    int r2 = r + r;

    int X  = r;
    int Y  = 0;
    int dY = -2;
    int dX = r2 + r2 - 4;
    int D  = r2 - 1;

    while (Y <= X) {

        FillLine(BitMap, Cx, Cy, -X, Y, c);
        FillLine(BitMap, Cx, Cy, -Y, X, c);
        FillLine(BitMap, Cx, Cy, X, Y, c);
        FillLine(BitMap, Cx, Cy, Y, X, c);

        D += dY;
        dY -= 4;
        ++Y;

        if (D < 0) {
            D += dX;
            dX -= 4;
            --X;
        }
    }
}

internal void
BLine(CanvasBitMap* BitMap, int32 x0, int32 y0, int32 x1, int32 y1, Color c, uint32 width) {

    int32 dx = ((x1 - x0) > 0) ? (x1 - x0) : (x0 - x1);
    int32 dy = ((y1 - y0) > 0) ? (y0 - y1) : (y1 - y0);

    int32 sx = (x0 < x1) ? 1 : -1;
    int32 sy = (y0 < y1) ? 1 : -1;

    int32 e = dx + dy;

    while (true) {

        DrawSquareC(BitMap, x0, y0, c, width);

        if ((x0 == x1) && (y0 == y1)) {
            break;
        }

        int32 e2 = 2 * e;
        if (e2 >= dy) {
            if (x0 == x1) {
                break;
            }

            e  = e + dy;
            x0 = x0 + sx;
        }

        if (e2 <= dx) {
            if (y0 == y1) {
                break;
            }

            e  = e + dx;
            y0 = y0 + sy;
        }
    }
}

// ---------------------------------------------------------------------------------------------- //
#endif
