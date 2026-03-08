#ifndef RENDERER_H
#define RENDERER_H
// Renderer interface to be used by the game and all renderer backends ------------------------- //

#include "base/sugars.hpp"

Enum(render_group_kind, RCK_Clear, RCK_Cube, RCK_CubeWF);

typedef struct {
    v4 color;
} RC_clear2d;

typedef struct {
    v3 pos;
    v3 scale;
    v4 rotation;
    v4 color;
} RC_cube;

typedef struct {
    v3 pos;
    v3 scale;
    v4 rotation;
    v4 color;
} RC_cube_wf;

typedef struct {
    RC_cube* cubes;
    u32      size;
    u32      count;
} RG_cube;

typedef struct {
    RC_cube_wf* cubes;
    u32         size;
    u32         count;
} RG_cube_wf;

typedef struct {
    f32 fov;
    v3  pos;
    v4  orientation;
} render_camera;

typedef struct {
    RG_cube       cube_buffer;
    RG_cube_wf    cube_wf_buffer;
    RC_clear2d    clear;
    render_camera camera;
} render_push_buffer;


// Platform implementation declarations
#define RNDR_ALLOCATE_PUSH_BUFFER(name) void name(render_push_buffer* push_buffer)
RNDR_ALLOCATE_PUSH_BUFFER(AllocatePushBuffer);

#define RNDR_CLEAR_PUSH_BUFFER(name) void name(render_push_buffer* push_buffer)
RNDR_CLEAR_PUSH_BUFFER(ClearPushBuffer);



// Self implementation declarations
#define PUSH_CLEAR(name) void name(render_push_buffer* push_buffer, RC_clear2d clear)
PUSH_CLEAR(PushClear);

#define PUSH_CUBE(name) void name(render_push_buffer* push_buffer, RC_cube cube)
PUSH_CUBE(PushCube);

#define PUSH_CUBE_WF(name) void name(render_push_buffer* push_buffer, RC_cube_wf cube_wf)
PUSH_CUBE_WF(PushCubeWF);


// Renderer implementation declarations
#define RNDR_INIT_FRAME(name) void name(render_push_buffer push_buffer)
RNDR_INIT_FRAME(InitFrame);

#define RNDR_RENDER(name) void name(render_push_buffer push_buffer)
RNDR_RENDER(Render);


// Renderer interface to be used by the game and all renderer backends ------------------------- //
#endif
