#ifndef RENDERER_H
#define RENDERER_H
//  Renderer interfaces : ------------------------------------------------------------ (section)  //

#include "base/include.cpp"

Enum(render_group_kind, RCK_Clear, RCK_Cube, RCK_CubeWF);

typedef struct {
    v4 color;
} RC_clear2d;

typedef struct {
    v3   pos;
    v3   scale;
    quat rotation;
    v4   color;
} RC_cube;

typedef struct {
    v3   pos;
    v3   scale;
    quat rotation;
    v4   color;
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
    f32  fov;
    v3   pos;
    quat orientation;
} render_camera;

typedef struct {
    RG_cube       cube_buffer;
    RG_cube_wf    cube_wf_buffer;
    RC_clear2d    clear;
    render_camera camera;
} render_push_buffer;

//  platform layer declares : -------------------------------------------------------- (section)  //
#define RNDR_ALLOCATE_PUSH_BUFFER(name) void name(render_push_buffer* push_buffer)
RNDR_ALLOCATE_PUSH_BUFFER(AllocatePushBuffer);

#define RNDR_CLEAR_PUSH_BUFFER(name) void name(render_push_buffer* push_buffer)
RNDR_CLEAR_PUSH_BUFFER(ClearPushBuffer);
//  (section) -------------------------------------------------------- : platform layer declares  //



//  renderer utils : ----------------------------------------------------------------- (section)  //
#define PUSH_CLEAR(name) void name(render_push_buffer* push_buffer, RC_clear2d clear)
PUSH_CLEAR(PushClear);

#define PUSH_CUBE(name) void name(render_push_buffer* push_buffer, RC_cube cube)
PUSH_CUBE(PushCube);

#define PUSH_CUBE_WF(name) void name(render_push_buffer* push_buffer, RC_cube_wf cube_wf)
PUSH_CUBE_WF(PushCubeWF);
//  (section) ----------------------------------------------------------------- : renderer utils  //


//  renderer implementation declares : ----------------------------------------------- (section)  //
#define RNDR_INIT_FRAME(name) void name(render_push_buffer push_buffer)
RNDR_INIT_FRAME(InitFrame);

#define RNDR_RENDER(name) void name(render_push_buffer push_buffer)
RNDR_RENDER(Render);
//  (section) ----------------------------------------------- : renderer implementation declares  //


//  (section) ------------------------------------------------------------ : Renderer interfaces  //
#endif
