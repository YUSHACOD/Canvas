
#include "opengl.hpp"
#include "windows.hpp"
#include "windows_debug.hpp"

#include "windows_debug.cpp"

#include <cmath>

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
    glVertexAttrib4fv    = (gl_vertex_attrib4fv*)wglGetProcAddress("glVertexAttrib4fv");
    glVertexAttrib1f     = (gl_vertex_attrib1f*)wglGetProcAddress("glVertexAttrib1f");
    glUniformMatrix4fv   = (gl_uniform_matrix4fv*)wglGetProcAddress("glUniformMatrix4fv");
    glGetUniformLocation = (gl_get_uniform_location*)wglGetProcAddress("glGetUniformLocation");
    glGetProgramiv       = (gl_get_programiv*)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog  = (gl_get_program_info_log*)wglGetProcAddress("glGetProgramInfoLog");
    glValidateProgram    = (gl_validate_program*)wglGetProcAddress("glValidateProgram");
}

internal void GLInit(HWND window_handle) {

    PIXELFORMATDESCRIPTOR pixel_format_desc = {};
    pixel_format_desc.nSize                 = sizeof(PIXELFORMATDESCRIPTOR);
    pixel_format_desc.nVersion              = 1;
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

internal void GLPipeLineSetup(gl_renderer_state* gl_state, f32 aspect_ratio) {

    GLLoadPrograms(gl_state);

    // Vertex Array Object Creation
    glCreateVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
    glBindVertexArray(gl_state->vao_handle);

    // Projection Matrix
    // f32 fov    = 1.0f;
    f32 fov = 1.0f / tan(Pi32 / 4.0f);

    // PLEASE REMEMBER THIS -------------------------------
    // Even if the z_near and z_far
    // are negative values, it doesn't mean
    // that the co-ordinate space for z will be negative.
    // the co-ordinate space is always positive.
    // PLEASE REMEMBER THIS -------------------------------
    f32 z_near = -0.1f;
    f32 z_far  = -1000.f;
    f32 lmda   = z_far / (z_far - z_near);

    f32 c0r0 = fov / aspect_ratio;
    f32 c1r1 = fov;
    f32 c2r2 = -lmda;
    f32 c3r2 = lmda * z_near;
    // f32 c2r2 = 1.0f;
    // f32 c3r2 = 0.0f;

    // clang-format off
    f32 proj[16] = GL_MAT(
		c0r0,    0,    0,    0,
		   0, c1r1,    0,    0,
		   0,    0, c2r2, c3r2,
		   0,    0,   -1,    0 
	);

    f32 view[16] = GL_MAT(
		   1,    0,    0,    0,
		   0,    1,    0,    0,
		   0,    0,    1,    0,
		   0,    0,    0,    1 
	);

    f32 world[16] = GL_MAT(
		   1,    0,    0,    0,
		   0,    1,    0,    0,
		   0,    0,    1,    0,
		   0,    0,    0,    1 
	);

	f32* uniforms[EnumCount(uniform_kind)] = {0};
	uniforms[ProjMat]  = proj;
	uniforms[ViewMat]  = view;
	uniforms[WorldMat] = world;

    for EachEnumVal(shader_program_kind, shdr) {
		for EachEnumVal(uniform_kind, u) {

			gl_state->uniform_locations[u] = glGetUniformLocation(
					gl_state->program_handles[shdr], 
					gl_glbl_uniform_name[u]
			);

			// One has to load the program 
			// to load a uniform into it
			glUseProgram(gl_state->program_handles[shdr]);

			glUniformMatrix4fv(gl_state->uniform_locations[u], 1, GL_FALSE, uniforms[u]);

		}
    }
    // clang-format on

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

CANVAS_GL_CLEAR(GLClear) {
    glUseProgram(gl_state->program_handles[General]);

    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    glClearBufferfv(GL_COLOR, 0, (GLfloat*)color.pos);
}

CANVAS_GL_DRAW_CUBE(GLDrawCube) {
    glUseProgram(gl_state->program_handles[Cube]);

    glVertexAttrib4fv(3, (GLfloat*)color.pos);

    f32 lerp_offset = ((f32)sin(dt) * 0.5f + 0.5f);
    glVertexAttrib1f(4, lerp_offset);

    // glPointSize(20.0f);
    // glDrawArrays(GL_POINTS, 0, 1);

    glDrawArrays(GL_LINES, 0, 24);
}
