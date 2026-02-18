
#include "windows_opengl.hpp"
#include "windows_structs.hpp"

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

internal i32 GLPipeLineSetup(GLPipelineState*   gl_state,
                             const GLchar*      vertex_shader_source_,
                             const GLchar*      fragment_shader_source_,
                             f32 aspect_ratio) {

#if 0
    const GLchar* vertex_shader_source_ = {"#version 450 core                          \n"
                                           "                                           \n"
                                           "void main(void) {                          \n"
                                           "	gl_Position = vec4(0.0, 0.0, 0.5, 1.0);\n"
                                           "}                                          \n"};

    const GLchar* fragment_shader_source_ = {"#version 450 core                    \n"
                                             "                                     \n"
                                             "out vec4 color;                      \n"
                                             "                                     \n"
                                             "void main(void)  {                   \n"
                                             "    color = vec4(1.0, 1.0, 1.0, 1.0);\n"
                                             "}                                    \n"};
#endif

    const GLchar* vertex_shader_source[1];
    vertex_shader_source[0] = vertex_shader_source_;

    const GLchar* fragment_shader_source[1];
    fragment_shader_source[0] = fragment_shader_source_;

    GLuint vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

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

    gl_state->program_handle = glCreateProgram();
    glAttachShader(gl_state->program_handle, vertex_shader);
    glAttachShader(gl_state->program_handle, fragment_shader);

    glLinkProgram(gl_state->program_handle);

    gl_state->projection_location = glGetUniformLocation(gl_state->program_handle, "proj");

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Vertex Array Object Creation
    glCreateVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
    glBindVertexArray(gl_state->vao_handle);

    f32 proj[16] = {0};
    proj[0]      = 1.0f / aspect_ratio;
    proj[5]      = 1.0f;
    proj[10]     = 1.0f;
    proj[11]     = 1.0f;
    proj[15]     = 1.0f;

    glUseProgram(gl_state->program_handle);
    glUniformMatrix4fv(gl_state->projection_location, 1, GL_FALSE, proj);
    return 0;
}

internal void GlPipelineDelete(GLPipelineState* gl_state) {
    glDeleteProgram(gl_state->program_handle);
    glDeleteVertexArrays(gl_state->vao_len, &gl_state->vao_handle);
}

internal void GLFixProjection(GLPipelineState*  gl_state,
                              WinPlatDimensions window,
                              WinPlatDimensions display,
                              f32               display_aspect) {

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
