#ifndef CANVXS_GAME_H
#define CANVXS_GAME_H
//  Game related stuff : ------------------------------------------------------------- (section)  //

#include "base/sugars.hpp"

typedef struct {
    f32 jx;
    f32 jy;

    f32 x_off;
    f32 y_off;
    f32 z_off;

    f32 weight;

    f32 dx;
    f32 dy;
    f32 dz;

    f32 theta;
} canvas_state;

//  (section) ------------------------------------------------------------- : Game related stuff  //
#endif
