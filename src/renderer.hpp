#ifndef RENDERER_H
#define RENDERER_H
// Renderer interface to be used by the game and all renderer backends ------------------------- //

#include "base/sugars.hpp"

Enum(render_group_kind, RCK_Clear, RCK_Cube, RCK_CubeWF);

typedef struct {
    v4 color;
} RC_clear2d;

typedef struct {
    v4  pos;
    v4  color;
    f32 lerp_offset;
} RC_cube;

typedef struct {
    v4  pos;
    v4  color;
    f32 lerp_offset;
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
    RG_cube    cube;
    RG_cube_wf cube_wf;
    RC_clear2d clear;
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
#define RNDR_CLEAR(name) void name(RC_clear2d cmd)
RNDR_CLEAR(RenderClear);

#define RNDR_CUBES(name) void name(RG_cube rg)
RNDR_CUBES(RenderCubes);

#define RNDR_CUBES_WF(name) void name(RG_cube_wf rg)
RNDR_CUBES_WF(RenderCubesWF);

#define RNDR_RENDER(name) void name(render_push_buffer push_buffer)
RNDR_RENDER(Render);


// Renderer interface to be used by the game and all renderer backends ------------------------- //
#endif
