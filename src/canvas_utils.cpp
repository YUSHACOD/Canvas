#ifndef CANVAS_UTILS_H
#define CANVAS_UTILS_H
// ---------------------------------------------------------------------------------------------- //

#include "canvas.hpp"

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Point3D;

typedef struct {
    f32 x;
    f32 y;
} Point;


// Colors --------------------------------------------------------------------------------------- //
typedef struct {
    u8 red;
    u8 green;
    u8 blue;
    u8 pad;
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

inline internal void DrawPixel(u32* draw_pixel, u8 red, u8 green, u8 blue, u8 pad) {
    *draw_pixel = ((u32)pad << 24) | ((u32)red << 16) | ((u32)green << 8) | ((u32)blue);
}

inline internal void
DrawPixel(CanvasBitMap* bitmap, i32 x, i32 y, u8 red, u8 blue, u8 green, u8 pad) {

    u32 dx = x + (bitmap->Width / 2);
    u32 dy = -y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        u8*  draw_row   = ((u8*)bitmap->Memory) + (bitmap->Pitch * dy);
        u32* draw_pixel = ((u32*)draw_row + dx);

        *draw_pixel = ((u32)pad << 24) | ((u32)red << 16) | ((u32)green << 8) | ((u32)blue);
    }
}

internal void DrawPixel(CanvasBitMap* bitmap, i32 x, i32 y, Color c) {

    u32 dx = x + (bitmap->Width / 2);
    u32 dy = -y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        u8*  draw_row   = ((u8*)bitmap->Memory) + (bitmap->Pitch * dy);
        u32* draw_pixel = ((u32*)draw_row + dx);

        *draw_pixel = ((u32)c.pad << 24) | ((u32)c.red << 16) | ((u32)c.green << 8) | ((u32)c.blue);
    }
}

internal void DrawRectangle(CanvasBitMap* bitmap, i32 x, i32 y, Color c, u32 width, u32 height) {

    for (u32 draw_y = 0; draw_y < height; draw_y += 1) {
        for (u32 draw_x = 0; draw_x < width; draw_x += 1) {
            DrawPixel(bitmap, draw_x + x, draw_y + y, c);
        }
    }
}

inline internal void DrawSquare(CanvasBitMap* bitmap, i32 x, i32 y, Color c, u32 size) {
    DrawRectangle(bitmap, x, y, c, size, size);
}

inline internal void DrawSquareC(CanvasBitMap* BitMap, i32 X, i32 Y, Color c, u32 size) {
    DrawSquare(BitMap, X - (size / 2), Y - (size / 2), c, size);
}

void CaseyCircle(CanvasBitMap* BitMap, i32 Cx, i32 Cy, u32 r, Color c) {
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

internal void FillLine(CanvasBitMap* bitmap, i32 Cx, i32 Cy, i32 x, i32 y, Color c) {
    for (i32 i = y; i >= -y; i -= 1) {
        DrawPixel(bitmap, Cx - x, Cy - i, c);
    }
}

internal void CaseyCircleFill(CanvasBitMap* BitMap, i32 Cx, i32 Cy, u32 r, Color c) {
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

internal void BLine(CanvasBitMap* BitMap, i32 x0, i32 y0, i32 x1, i32 y1, Color c, u32 width) {

    i32 dx = ((x1 - x0) > 0) ? (x1 - x0) : (x0 - x1);
    i32 dy = ((y1 - y0) > 0) ? (y0 - y1) : (y1 - y0);

    i32 sx = (x0 < x1) ? 1 : -1;
    i32 sy = (y0 < y1) ? 1 : -1;

    i32 e = dx + dy;

    while (true) {


        DrawSquareC(BitMap, x0, y0, c, width);

        if ((x0 == x1) && (y0 == y1)) {
            break;
        }

        i32 e2 = 2 * e;
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
