
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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
    glGenerateMipmap     = (gl_generate_mipmap*)wglGetProcAddress("glGenerateMipmap");
}
//  (section) --------------------------------------------------------------- : loading gl funcs  //


//  opengl init : -------------------------------------------------------------------- (section)  //
internal void GL_Init(HWND window_handle) {

    // NOTE: Bootstrap a dummy context to load WGL extension functions,
    //       which are required to create a Core Profile context for RenderDoc.
    {
        HWND dummy_window = CreateWindowExA(0, "STATIC", "", WS_POPUP, 0, 0, 1, 1, 0, 0, 0, 0);
        HDC  dummy_dc     = GetDC(dummy_window);

        PIXELFORMATDESCRIPTOR dummy_pfd = {};
        dummy_pfd.nSize                 = sizeof(PIXELFORMATDESCRIPTOR);
        dummy_pfd.nVersion              = 1;
        dummy_pfd.dwFlags               = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
        dummy_pfd.iPixelType            = PFD_TYPE_RGBA;
        dummy_pfd.cColorBits            = 32;

        SetPixelFormat(dummy_dc, ChoosePixelFormat(dummy_dc, &dummy_pfd), &dummy_pfd);

        HGLRC dummy_ctx = wglCreateContext(dummy_dc);
        wglMakeCurrent(dummy_dc, dummy_ctx);

        wglChoosePixelFormatARB =
            (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB =
            (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");

        wglMakeCurrent(dummy_dc, 0);
        wglDeleteContext(dummy_ctx);
        ReleaseDC(dummy_window, dummy_dc);
        DestroyWindow(dummy_window);
    }

    HDC device_ctx = GetDC(window_handle);

    // NOTE: Must use wglChoosePixelFormatARB (not ChoosePixelFormat) when
    //       going through the ARB context creation path.
    const i32 pf_attribs[] = {0x2001,
                              1, // WGL_DRAW_TO_WINDOW_ARB
                              0x2010,
                              1, // WGL_SUPPORT_OPENGL_ARB
                              0x2011,
                              1, // WGL_DOUBLE_BUFFER_ARB
                              0x2013,
                              0x202B, // WGL_PIXEL_TYPE_ARB = WGL_TYPE_RGBA_ARB
                              0x2014,
                              32, // WGL_COLOR_BITS_ARB
                              0x2022,
                              24, // WGL_DEPTH_BITS_ARB
                              0x2023,
                              8, // WGL_STENCIL_BITS_ARB
                              0};

    i32  pixel_fmt_idx = 0;
    UINT num_formats   = 0;
    wglChoosePixelFormatARB(device_ctx, pf_attribs, 0, 1, &pixel_fmt_idx, &num_formats);

    PIXELFORMATDESCRIPTOR pixel_fmt_desc_final = {};
    DescribePixelFormat(
        device_ctx, pixel_fmt_idx, sizeof(PIXELFORMATDESCRIPTOR), &pixel_fmt_desc_final);
    SetPixelFormat(device_ctx, pixel_fmt_idx, &pixel_fmt_desc_final);

    // NOTE: Core Profile flag is the critical piece RenderDoc hooks into.
    const i32 ctx_attribs[] = {0x2091,
                               4, // WGL_CONTEXT_MAJOR_VERSION_ARB
                               0x2092,
                               6, // WGL_CONTEXT_MINOR_VERSION_ARB
                               0x9126,
                               0x00000001, // WGL_CONTEXT_PROFILE_MASK_ARB  = CORE
                               0x2094,
                               0x00000002, // WGL_CONTEXT_FLAGS_ARB         = DEBUG
                               0};

    HGLRC rendering_context = wglCreateContextAttribsARB(device_ctx, 0, ctx_attribs);
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


internal void GL_Release(HWND window_handle) {

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
internal char* GL_load_shader_source(shader_program_kind kind, char* suffix) {
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

internal void GL_load_shaders(gl_renderer_state* gl_state) {
        for
            EachEnumVal(shader_program_kind, idx) {
                GLchar* vertex_source;
                GLchar* fragment_source;
                {
                    char vert_suffix[] = "_vertex.glsl";
                    vertex_source      = GL_load_shader_source(idx, vert_suffix);
                }
                {
                    char frag_suffix[] = "_fragment.glsl";
                    fragment_source    = GL_load_shader_source(idx, frag_suffix);
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

internal void GL_load_textures(gl_renderer_state* rs) {
    glGenTextures(1, &rs->texture_handle);
    glBindTexture(GL_TEXTURE_2D, rs->texture_handle);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the texture
    i32 width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    // escher.png
    u8* data = stbi_load("../../res/escher.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        OutputDebugStringA("Failed to load texture");
    }
    stbi_image_free(data);
}

internal void GL_PipeLineSetup(gl_renderer_state* gl_state, f32 aspect_ratio, GLuint vao_len) {

    ogl_state.vao_len      = vao_len;
    ogl_state.aspect_ratio = aspect_ratio;
    GL_load_shaders(gl_state);

    // Vertex Array Object Creation
    glCreateVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
    glBindVertexArray(gl_state->vao_handle);

    GL_load_textures(gl_state);

    glEnable(GL_DEPTH_TEST);
    gl_state->is_valid = true;
}

// clang-format off
internal void GL_PipelineDelete(gl_renderer_state* gl_state) {
	for EachEnumVal(shader_program_kind, idx) { 
		glDeleteProgram(gl_state->program_handles[idx]);
	}

    glDeleteVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
}
// clang-format on

//  (section) --------------------------------------------------------------- : gl pipeline init  //



//  frame setup : -------------------------------------------------------------------- (section)  //
internal void
GL_load_projection_matrix(f32* proj, f32 aspect_ratio, f32 fov_angle_radians, f32 z_near, f32 z_far) {

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

internal void GL_load_view_matrix(f32* view_transform, v3 pos, quat orientation) {

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

inline internal void GL_load_view_matrix(mat4 *v, render_camera* cam) {

    V3_veci(v->vec1, cam->side.x, cam->up.x, -cam->front.x);
    V3_veci(v->vec2, cam->side.y, cam->up.y, -cam->front.y);
    V3_veci(v->vec3, cam->side.z, cam->up.z, -cam->front.z);

    V3_veci(v->vec4, -(cam->side * cam->pos), -(cam->up * cam->pos), (cam->front * cam->pos));
}

internal void GL_FixProjection(gl_renderer_state* gl_state, winplat_dimensions window) {

    f32 window_aspect_ratio = (f32)window.width / (f32)window.height;
    gl_state->aspect_ratio  = window_aspect_ratio;

    glViewport(0, 0, window.width, window.height);
}

internal void GL_init_frame(render_push_buffer* push_buffer) {

    glUseProgram(ogl_state.program_handles[General]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    f32 proj[16] = {};
#define Z_NEAR -0.1f
#define Z_FAR  -1000.0f
    GL_load_projection_matrix(proj, ogl_state.aspect_ratio, DegToRad(60.0f), Z_NEAR, Z_FAR);

    Mat4_di(view_mat);
	GL_load_view_matrix(&view_mat, &push_buffer->cam);

    f32* uniforms[EnumCount(uniform_kind)] = {0};
    uniforms[ProjMat]                      = proj;
    uniforms[ViewMat]                      = view_mat.arr;

    // clang-format off
    for EachEnumVal(shader_program_kind, shdr) {
		for EachEnumVal(uniform_kind, u) {

			// One has to load the program to load a uniform into it
			glUseProgram(ogl_state.program_handles[shdr]);
			glUniformMatrix4fv(u, 1, GL_FALSE, uniforms[u]);
		}
    }
    // clang-format on
}
//  (section) -------------------------------------------------------------------- : frame setup  //



//  Renderer utils : ----------------------------------------------------------------- (section)  //
internal void GL_render_clear(RC_clear2d cmd) {

    glUseProgram(ogl_state.program_handles[General]);
    glClearBufferfv(GL_COLOR, 0, (GLfloat*)cmd.color.arr);
}


internal void GL_render_cubes(RG_cube rg) {
    glUseProgram(ogl_state.program_handles[Cube]);

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

internal void GL_render_cubes_wf(RG_cube_wf rg) {
    glUseProgram(ogl_state.program_handles[CubeWireFrame]);

    for (u32 idx = 0; idx < rg.count; idx++) {

        glVertexAttrib3fv(0, (GLfloat*)rg.cubes[idx].pos.arr);
        glVertexAttrib3fv(1, (GLfloat*)rg.cubes[idx].scale.arr);
        glVertexAttrib4fv(2, (GLfloat*)rg.cubes[idx].rotation.arr);

        glVertexAttrib4fv(3, (GLfloat*)rg.cubes[idx].color.arr);

        glDrawArrays(GL_LINES, 0, 24);
    }
}

R_RENDER(R_Render) {
    GL_init_frame(push_buffer);

    GL_render_clear(push_buffer->clear);
    GL_render_cubes(push_buffer->cube_buffer);
    GL_render_cubes_wf(push_buffer->cube_wf_buffer);
}
//  (section) ----------------------------------------------------------------- : Renderer utils  //
