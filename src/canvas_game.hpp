#ifndef CANVXS_GAME_H
#define CANVXS_GAME_H
//  Game related stuff : ------------------------------------------------------------- (section)  //

#include "base/include.cpp"

#define PIECE_SIZE      20.0f
#define PIECE_MOVE_TIME 500.f

typedef struct {
    u8 x;
    u8 y;
    u8 z;
} GridPos;

typedef struct {
    f32     t;
    v3 prev_position;
    bool    active;
} PieceAnimation;

typedef struct {
	GridPos pos;
} Piece;

typedef struct {
    u8  rows;
    u8  cols;
    u8  layers;
    v3  pos;
    f32 cube_size;
} Grid;

typedef struct {

#if DEBUG
    f32 x_off;
    f32 y_off;
    f32 z_off;

    f32 dt;
#endif

    Grid grid;

    Piece          piece;
    PieceAnimation p_anim;

    bool debug_camera_mode;
} canvas_state;

//  (section) ------------------------------------------------------------- : Game related stuff  //
#endif
