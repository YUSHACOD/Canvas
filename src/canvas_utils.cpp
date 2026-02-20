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
DrawPixel(canvas_bitmap* bitmap, i32 x, i32 y, u8 red, u8 blue, u8 green, u8 pad) {

    u32 dx = x + (bitmap->width / 2);
    u32 dy = -y + (bitmap->height / 2);

    if ((dx < bitmap->width) && (dy < bitmap->height)) {

        u8*  draw_row   = ((u8*)bitmap->memory) + (bitmap->pitch * dy);
        u32* draw_pixel = ((u32*)draw_row + dx);

        *draw_pixel = ((u32)pad << 24) | ((u32)red << 16) | ((u32)green << 8) | ((u32)blue);
    }
}

internal void DrawPixel(canvas_bitmap* bitmap, i32 x, i32 y, Color c) {

    u32 dx = x + (bitmap->width / 2);
    u32 dy = -y + (bitmap->height / 2);

    if ((dx < bitmap->width) && (dy < bitmap->height)) {

        u8*  draw_row   = ((u8*)bitmap->memory) + (bitmap->pitch * dy);
        u32* draw_pixel = ((u32*)draw_row + dx);

        *draw_pixel = ((u32)c.pad << 24) | ((u32)c.red << 16) | ((u32)c.green << 8) | ((u32)c.blue);
    }
}

internal void DrawRectangle(canvas_bitmap* bitmap, i32 x, i32 y, Color c, u32 width, u32 height) {

    for (u32 draw_y = 0; draw_y < height; draw_y += 1) {
        for (u32 draw_x = 0; draw_x < width; draw_x += 1) {
            DrawPixel(bitmap, draw_x + x, draw_y + y, c);
        }
    }
}

inline internal void DrawSquare(canvas_bitmap* bitmap, i32 x, i32 y, Color c, u32 size) {
    DrawRectangle(bitmap, x, y, c, size, size);
}

inline internal void DrawSquareC(canvas_bitmap* bitmap, i32 x, i32 y, Color c, u32 size) {
    DrawSquare(bitmap, x - (size / 2), y - (size / 2), c, size);
}

void CaseyCircle(canvas_bitmap* bitmap, i32 cx, i32 cy, u32 r, Color c) {
    int r2 = r + r;

    int x  = r;
    int y  = 0;
    int dy = -2;
    int dx = r2 + r2 - 4;
    int d  = r2 - 1;

    while (y <= x) {

        DrawPixel(bitmap, cx - x, cy - y, c);
        DrawPixel(bitmap, cx + x, cy - y, c);
        DrawPixel(bitmap, cx - x, cy + y, c);
        DrawPixel(bitmap, cx + x, cy + y, c);
        DrawPixel(bitmap, cx - y, cy - x, c);
        DrawPixel(bitmap, cx + y, cy - x, c);
        DrawPixel(bitmap, cx - y, cy + x, c);
        DrawPixel(bitmap, cx + y, cy + x, c);

        d += dy;
        dy -= 4;
        ++y;

        if (d < 0) {
            d += dx;
            dx -= 4;
            --x;
        }
    }
}

internal void FillLine(canvas_bitmap* bitmap, i32 Cx, i32 Cy, i32 x, i32 y, Color c) {
    for (i32 i = y; i >= -y; i -= 1) {
        DrawPixel(bitmap, Cx - x, Cy - i, c);
    }
}

internal void CaseyCircleFill(canvas_bitmap* bitmap, i32 cx, i32 cy, u32 r, Color c) {
    int r2 = r + r;

    int x  = r;
    int y  = 0;
    int dy = -2;
    int dx = r2 + r2 - 4;
    int d  = r2 - 1;

    while (y <= x) {
        FillLine(bitmap, cx, cy, -x, y, c);
        FillLine(bitmap, cx, cy, -y, x, c);
        FillLine(bitmap, cx, cy, x, y, c);
        FillLine(bitmap, cx, cy, y, x, c);

        d += dy;
        dy -= 4;
        ++y;

        if (d < 0) {
            d += dx;
            dx -= 4;
            --x;
        }
    }
}

internal void BLine(canvas_bitmap* bitmap, i32 x0, i32 y0, i32 x1, i32 y1, Color c, u32 width) {

    i32 dx = ((x1 - x0) > 0) ? (x1 - x0) : (x0 - x1);
    i32 dy = ((y1 - y0) > 0) ? (y0 - y1) : (y1 - y0);

    i32 sx = (x0 < x1) ? 1 : -1;
    i32 sy = (y0 < y1) ? 1 : -1;

    i32 e = dx + dy;

    while (true) {


        DrawSquareC(bitmap, x0, y0, c, width);

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

internal void ClearBitMap(canvas_bitmap* bitmap, u32 x_off, u32 y_off) {

    u8* row = (u8*)bitmap->memory;

    for (u32 y = 0; y < bitmap->height; y += 1) {
        u32* pixel = (u32*)row;
        for (u32 x = 0; x < bitmap->width; x += 1) {

            // Blue
            // uint8 blue  = (uint8)(X + x_off);
            // uint8 green = (uint8)(Y + y_off);
            u8 blue  = 34;
            u8 green = 34;
            u8 red   = 34;
            u8 pad   = 0;

            DrawPixel(pixel, red, blue, green, pad);

            pixel += 1;
        }

        row += bitmap->pitch;
    }
}

// ---------------------------------------------------------------------------------------------- //
#endif
