#ifndef RENDERER_H
#define RENDERER_H
//  Renderer interfaces : ------------------------------------------------------------ (section)  //

#include <base/include.cpp>

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
    RG_cube    cube_buffer;
    RG_cube_wf cube_wf_buffer;
    RC_clear2d clear;
    mat4       view_mat;
} render_push_buffer;

//  platform layer declares : -------------------------------------------------------- (section)  //
#define R_ALLOCATE_PUSH_BUFFER(name) void name(render_push_buffer* push_buffer)
R_ALLOCATE_PUSH_BUFFER(R_AllocatePushBuffer);

#define R_CLEAR_PUSH_BUFFER(name) void name(render_push_buffer* push_buffer)
R_CLEAR_PUSH_BUFFER(R_ClearPushBuffer);
//  (section) -------------------------------------------------------- : platform layer declares  //



//  renderer utils : ----------------------------------------------------------------- (section)  //
#define R_PUSH_CLEAR(name) void name(render_push_buffer* push_buffer, RC_clear2d clear)
R_PUSH_CLEAR(R_PushClear);

#define R_PUSH_CUBE(name) void name(render_push_buffer* push_buffer, RC_cube cube)
R_PUSH_CUBE(R_PushCube);

#define R_PUSH_CUBE_WF(name) void name(render_push_buffer* push_buffer, RC_cube_wf cube_wf)
R_PUSH_CUBE_WF(R_PushCubeWF);
//  (section) ----------------------------------------------------------------- : renderer utils  //


//  renderer implementation declares : ----------------------------------------------- (section)  //
#define R_RENDER(name) void name(render_push_buffer* push_buffer)
R_RENDER(R_Render);
//  (section) ----------------------------------------------- : renderer implementation declares  //


//  (section) ------------------------------------------------------------ : Renderer interfaces  //
#endif
