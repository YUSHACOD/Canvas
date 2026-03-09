
#include "opengl.hpp"
#include "renderer.hpp"
#include "windows.hpp"
#include "windows_debug.hpp"

#include "windows_debug.cpp"

#include <cmath>


//  loading gl funcs : --------------------------------------------------------------- (section)  //
internal void GL_load_function_globals() {
    glCreateShader       = (gl_create_shader*)wglGetProcAddress("glCreateShader");
    glShaderSource       = (gl_shader_source*)wglGetProcAddress("glShaderSource");
    glCompileShader      = (gl_compile_shader*)wglGetProcAddress("glCompileShader");
    glCreateProgram      = (gl_create_program*)wglGetProcAddress("glCreateProgram");
    glAttachShader       = (gl_attach_shader*)wglGetProcAddress("glAttachShader");
    glLinkProgram        = (gl_link_program*)wglGetProcAddress("glLinkProgram");
    glDeleteShader       = (gl_delete_shader*)wglGetProcAddress("glDeleteShader");
    glDeleteProgram      = (gl_delete_program*)wglGetProcAddress("glDeleteProgram");
    glCreateVertexArrays = (gl_create_vertex_arrays*)wglGetProcAddress("glCreateVertexArrays");
    glBindVertexArray    = (gl_bind_vertex_array*)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (gl_delete_vertex_arrays*)wglGetProcAddress("glDeleteVertexArrays");
    glUseProgram         = (gl_use_program*)wglGetProcAddress("glUseProgram");
    glGetShaderiv        = (gl_get_shaderiv*)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog   = (gl_get_shader_info_log*)wglGetProcAddress("glGetShaderInfoLog");
    glClearBufferfv      = (gl_clear_bufferfv*)wglGetProcAddress("glClearBufferfv");
    glVertexAttrib3fv    = (gl_vertex_attrib3fv*)wglGetProcAddress("glVertexAttrib3fv");
    glVertexAttrib4fv    = (gl_vertex_attrib4fv*)wglGetProcAddress("glVertexAttrib4fv");
    glVertexAttrib1f     = (gl_vertex_attrib1f*)wglGetProcAddress("glVertexAttrib1f");
    glUniformMatrix4fv   = (gl_uniform_matrix4fv*)wglGetProcAddress("glUniformMatrix4fv");
    glGetUniformLocation = (gl_get_uniform_location*)wglGetProcAddress("glGetUniformLocation");
    glGetProgramiv       = (gl_get_programiv*)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog  = (gl_get_program_info_log*)wglGetProcAddress("glGetProgramInfoLog");
    glValidateProgram    = (gl_validate_program*)wglGetProcAddress("glValidateProgram");
}
//  (section) --------------------------------------------------------------- : loading gl funcs  //


//  opengl init : -------------------------------------------------------------------- (section)  //
internal void GLInit(HWND window_handle) {

    PIXELFORMATDESCRIPTOR pixel_format_desc = {};

    pixel_format_desc.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format_desc.nVersion   = 1;
    pixel_format_desc.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixel_format_desc.iPixelType = PFD_TYPE_RGBA;
    pixel_format_desc.cColorBits = 24;
    pixel_format_desc.cRedBits   = 8;
    pixel_format_desc.cGreenBits = 8;
    pixel_format_desc.cBlueBits  = 8;
    pixel_format_desc.cAlphaBits = 8;

    HDC device_ctx    = GetDC(window_handle);
    i32 pixel_fmt_idx = ChoosePixelFormat(device_ctx, &pixel_format_desc);

    PIXELFORMATDESCRIPTOR pixel_fmt_desc_final = {};
    DescribePixelFormat(
        device_ctx, pixel_fmt_idx, sizeof(PIXELFORMATDESCRIPTOR), &pixel_fmt_desc_final);

    SetPixelFormat(device_ctx, pixel_fmt_idx, &pixel_fmt_desc_final);

    HGLRC rendering_context = wglCreateContext(device_ctx);
    if (rendering_context) {
        if (wglMakeCurrent(device_ctx, rendering_context)) {

        } else {
            // TODO: what to do when it fails
        }
    }

    wglSwapIntervalEXT = (wgl_swap_interval_ext*)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(1);
    }

    GL_load_function_globals();
}


internal void GLDeInit(HWND window_handle) {

    HGLRC rendering_context = wglGetCurrentContext();

    if (rendering_context) {
        HDC device_ctx = wglGetCurrentDC();

        wglMakeCurrent(NULL, NULL);
        ReleaseDC(window_handle, device_ctx);
        wglDeleteContext(rendering_context);
    }
}
//  (section) -------------------------------------------------------------------- : opengl init  //


//  gl pipeline init : --------------------------------------------------------------- (section)  //
#if DEBUG
internal char* GLLoadShaderSource(shader_program_kind kind, char* suffix) {
    char buffer[512]  = "../../shaders/";
    i32  buff_pre_len = 14;
    i32  path_len;
    for (path_len = 0; gl_glbl_shader_program_prefixes[kind][path_len] != 0;
         (buffer[path_len + buff_pre_len] = gl_glbl_shader_program_prefixes[kind][path_len],
                            path_len++))
        ;
    path_len += buff_pre_len;
    for (i32 i = 0; suffix[i] != 0; (buffer[path_len++] = suffix[i++]))
        ;
    DBG_FileStruct vert_file = DBG_PlatReadEntireFile(buffer);

    return (char*)vert_file.memory;
}
#endif

internal void GLLoadPrograms(gl_renderer_state* gl_state) {
        for
            EachEnumVal(shader_program_kind, idx) {
                GLchar* vertex_source;
                GLchar* fragment_source;
                {
                    char vert_suffix[] = "_vertex.glsl";
                    vertex_source      = GLLoadShaderSource(idx, vert_suffix);
                }
                {
                    char frag_suffix[] = "_fragment.glsl";
                    fragment_source    = GLLoadShaderSource(idx, frag_suffix);
                }

                GLuint vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
                GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

                const GLchar* vertex_shader_source[1];
                vertex_shader_source[0] = vertex_source;

                const GLchar* fragment_shader_source[1];
                fragment_shader_source[0] = fragment_source;

                glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
                glCompileShader(vertex_shader);


                GLint vertex_compilation_status = 0;
                glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compilation_status);
                if (vertex_compilation_status != GL_TRUE) {
                    GLsizei log_length    = 0;
                    GLchar  message[1025] = {};
                    glGetShaderInfoLog(vertex_shader, 1024, &log_length, message);
                    OutputDebugStringA(message);
                    Assert(0);
                }

                glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
                glCompileShader(fragment_shader);

                GLint fragment_compilation_status = 0;
                glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compilation_status);
                if (fragment_compilation_status != GL_TRUE) {
                    GLsizei log_length    = 0;
                    GLchar  message[1025] = {};
                    glGetShaderInfoLog(fragment_shader, 1024, &log_length, message);
                    OutputDebugStringA(message);
                    Assert(0);
                }

                gl_state->program_handles[idx] = glCreateProgram();
                glAttachShader(gl_state->program_handles[idx], vertex_shader);
                glAttachShader(gl_state->program_handles[idx], fragment_shader);

                glLinkProgram(gl_state->program_handles[idx]);

                GLint linked;
                glGetProgramiv(gl_state->program_handles[idx], GL_LINK_STATUS, &linked);
                if (!linked) {
                    GLsizei log_length    = 0;
                    GLchar  message[1025] = {};
                    glGetProgramInfoLog(gl_state->program_handles[idx], 1024, &log_length, message);
                    OutputDebugStringA(message);
                    Assert(0);
                }

                glValidateProgram(gl_state->program_handles[idx]);
                GLint valid;
                glGetProgramiv(gl_state->program_handles[idx], GL_VALIDATE_STATUS, &valid);
                if (!valid) {
                    GLsizei log_length    = 0;
                    GLchar  message[1025] = {};
                    glGetProgramInfoLog(gl_state->program_handles[idx], 1024, &log_length, message);
                    OutputDebugStringA(message);
                    Assert(0);
                }

                glDeleteShader(vertex_shader);
                glDeleteShader(fragment_shader);

                DBG_PlatFreeFilememory(vertex_source);
                DBG_PlatFreeFilememory(fragment_source);
            }
}

internal void GLPipeLineSetup(gl_renderer_state* gl_state, f32 aspect_ratio, GLuint vao_len) {

    GLBL_opengl_state.vao_len      = vao_len;
    GLBL_opengl_state.aspect_ratio = aspect_ratio;
    GLLoadPrograms(gl_state);

    // Vertex Array Object Creation
    glCreateVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
    glBindVertexArray(gl_state->vao_handle);



    glEnable(GL_DEPTH_TEST);
    gl_state->is_valid = true;
}

// clang-format off
internal void GlPipelineDelete(gl_renderer_state* gl_state) {
	for EachEnumVal(shader_program_kind, idx) { 
		glDeleteProgram(gl_state->program_handles[idx]);
	}

    glDeleteVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
}
// clang-format on

//  (section) --------------------------------------------------------------- : gl pipeline init  //



//  frame setup : -------------------------------------------------------------------- (section)  //
internal void
GLLoadProjectionMatrix(f32* proj, f32 aspect_ratio, f32 fov_angle_radians, f32 z_near, f32 z_far) {

    f32 fov = 1.0f / tan(fov_angle_radians / 2.0f);

    // PLEASE REMEMBER THIS -------------------------------
    // - Even if the z_near and z_far
    // - are negative values, it doesn't mean
    // - that the co-ordinate space for z will be negative.
    // - the co-ordinate space is always positive.
    // PLEASE REMEMBER THIS -------------------------------

    f32 lmda = z_far / (z_far - z_near);

    f32 x_scale  = fov / aspect_ratio;
    f32 y_scale  = fov;
    f32 z_scale  = -lmda;
    f32 z_offset = lmda * z_near;
    f32 z_w_copy = -1.0f;

    proj[0]  = x_scale;
    proj[5]  = y_scale;
    proj[10] = z_scale;
    proj[11] = z_w_copy;
    proj[14] = z_offset;

    // proj = GL_MAT(
    // 	x_scale,       0,       0,       0,
    // 	      0, y_scale,       0,       0,
    // 	      0,       0, z_scale, z_offset,
    // 	      0,       0,      -1,       0
    // );
}

internal void GLLoadViewMatrix(f32* view_transform, v3 pos, v4 orientation) {

    f32 _00 = 1.0f;
    f32 _01 = 0.0f;
    f32 _02 = 0.0f;
    f32 _03 = 0.0f;
    f32 _04 = 0.0f;
    f32 _05 = 1.0f;
    f32 _06 = 0.0f;
    f32 _07 = 0.0f;
    f32 _08 = 0.0f;
    f32 _09 = 0.0f;
    f32 _10 = 1.0f;
    f32 _11 = 0.0f;
    f32 _12 = -pos.x;
    f32 _13 = -pos.y;
    f32 _14 = -pos.z;
    f32 _15 = 1.0f;

    // clang-format off
	view_transform[ 0]=_00; view_transform[ 4]=_04; view_transform[ 8]=_08; view_transform[12]=_12;
	view_transform[ 1]=_01; view_transform[ 5]=_05; view_transform[ 9]=_09; view_transform[13]=_13;
	view_transform[ 2]=_02; view_transform[ 6]=_06; view_transform[10]=_10; view_transform[14]=_14;
	view_transform[ 3]=_03; view_transform[ 7]=_07; view_transform[11]=_11; view_transform[15]=_15;
    // clang-format on
}

internal void GLFixProjection(gl_renderer_state* gl_state,
                              winplat_dimensions window,
                              winplat_dimensions display,
                              f32                display_aspect) {

    u32 dest_width  = window.width;
    u32 dest_height = window.height;

    f32 window_aspect_ratio = (f32)window.width / (f32)window.height;

    u32 dest_y = 0;
    u32 dest_x = 0;

    if (display_aspect >= window_aspect_ratio) {
        dest_y      = dest_height;
        dest_height = (u32)((f32)window.width / display_aspect);
        dest_y      = (dest_y - dest_height) / 2;
    } else {
        dest_x     = dest_width;
        dest_width = (u32)((f32)window.height * display_aspect);
        dest_x     = (dest_x - dest_width) / 2;
    }

    glViewport(dest_x, dest_y, dest_width, dest_height);
    glScissor(dest_x, dest_y, dest_width, dest_height);
}

RNDR_INIT_FRAME(InitFrame) {

    glUseProgram(GLBL_opengl_state.program_handles[General]);

    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);


    f32 proj[16] = {};
#define Z_NEAR -0.1f
#define Z_FAR  -1000.0f
    GLLoadProjectionMatrix(
        proj, GLBL_opengl_state.aspect_ratio, DegreestoRadians(60.0f), Z_NEAR, Z_FAR);

    f32 view[16];
    GLLoadViewMatrix(view, push_buffer.camera.pos, push_buffer.camera.orientation);

    f32* uniforms[EnumCount(uniform_kind)] = {0};
    uniforms[ProjMat]                      = proj;
    uniforms[ViewMat]                      = view;

    // clang-format off
    for EachEnumVal(shader_program_kind, shdr) {
		for EachEnumVal(uniform_kind, u) {

			// One has to load the program to load a uniform into it
			glUseProgram(GLBL_opengl_state.program_handles[shdr]);
			glUniformMatrix4fv(u, 1, GL_FALSE, uniforms[u]);
		}
    }
    // clang-format on
}
//  (section) -------------------------------------------------------------------- : frame setup  //



//  Renderer utils : ----------------------------------------------------------------- (section)  //
void RenderClear(RC_clear2d cmd) {

    glUseProgram(GLBL_opengl_state.program_handles[General]);
    glClearBufferfv(GL_COLOR, 0, (GLfloat*)cmd.color.arr);
}


void RenderCubes(RG_cube rg) {
    glUseProgram(GLBL_opengl_state.program_handles[Cube]);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (u32 idx = 0; idx < rg.count; idx++) {

        glVertexAttrib3fv(0, (GLfloat*)rg.cubes[idx].pos.arr);
        glVertexAttrib3fv(1, (GLfloat*)rg.cubes[idx].scale.arr);
        glVertexAttrib4fv(2, (GLfloat*)rg.cubes[idx].rotation.arr);

        glVertexAttrib4fv(3, (GLfloat*)rg.cubes[idx].color.arr);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderCubesWF(RG_cube_wf rg) {
    glUseProgram(GLBL_opengl_state.program_handles[CubeWireFrame]);

    for (u32 idx = 0; idx < rg.count; idx++) {

        glVertexAttrib3fv(0, (GLfloat*)rg.cubes[idx].pos.arr);
        glVertexAttrib3fv(1, (GLfloat*)rg.cubes[idx].scale.arr);
        glVertexAttrib4fv(2, (GLfloat*)rg.cubes[idx].rotation.arr);

        glVertexAttrib4fv(3, (GLfloat*)rg.cubes[idx].color.arr);

        glDrawArrays(GL_LINES, 0, 24);
    }
}

RNDR_RENDER(Render) {
    InitFrame(push_buffer);

    RenderClear(push_buffer.clear);
    RenderCubes(push_buffer.cube_buffer);
    RenderCubesWF(push_buffer.cube_wf_buffer);
}
//  (section) ----------------------------------------------------------------- : Renderer utils  //
