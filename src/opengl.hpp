#ifndef WIN_OPENGL_H
#define WIN_OPENGL_H
//  Opengl interfaces : -------------------------------------------------------------- (section)  //


#include <windows.h>
#include <GL\gl.h>

#include "base/interfaces.hpp"

typedef char GLchar;

//  gl defines : --------------------------------------------------------------------- (section)  //
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_VALIDATE_STATUS 0x8B83
//  (section) --------------------------------------------------------------------- : gl defines  //



//  generated includes : ------------------------------------------------------------- (section)  //
#include "generated/gl_api.c"
//  (section) ------------------------------------------------------------- : generated includes  //



Enum(shader_program_kind, Cube, CubeWireFrame, General);
global char* gl_glbl_shader_program_prefixes[EnumCount(shader_program_kind)] = {
    Text("cube"),
    Text("cube_wireframe"),
    Text("general"),
};

Enum(uniform_kind, ProjMat, ViewMat);
global char* gl_glbl_uniform_name[EnumCount(uniform_kind)] = {
    Text("proj"),
    Text("view"),
};

typedef struct {
    GLuint vao_len;
    GLuint vao_handle;
    f32    aspect_ratio;

    GLuint texture_handle;

    GLuint program_handles[EnumCount(shader_program_kind)];

    bool is_valid;
} gl_renderer_state;

global gl_renderer_state ogl_state;

//  (section) -------------------------------------------------------------- : Opengl interfaces  //
#endif
