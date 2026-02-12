
#include "windows_opengl.hpp"

internal void load_function_globals() {
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
}

internal void WinPlatInitGL(HWND window_handle) {

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

    load_function_globals();
}


internal void WinPlatDeInitGL(HWND window_handle) {

    HGLRC rendering_context = wglGetCurrentContext();

    if (rendering_context) {
        HDC device_ctx = wglGetCurrentDC();

        wglMakeCurrent(NULL, NULL);
        ReleaseDC(window_handle, device_ctx);
        wglDeleteContext(rendering_context);
    }
}

internal i32 WinPlatGLPipelineSetup(WinPlatGLPipelineState* ogl_state,
                                    const GLchar*           vertex_shader_source_,
                                    const GLchar*           fragment_shader_source_) {

    // const GLchar* vertex_shader_source_ = {"#version 450 core                          \n"
    //                                        "                                           \n"
    //                                        "void main(void) {                          \n"
    //                                        "	gl_Position = vec4(0.0, 0.0, 0.5, 1.0);\n"
    //                                        "}                                          \n"};
    //
    // const GLchar* fragment_shader_source_ = {"#version 450 core                    \n"
    //                                          "                                     \n"
    //                                          "out vec4 color;                      \n"
    //                                          "                                     \n"
    //                                          "void main(void)  {                   \n"
    //                                          "    color = vec4(1.0, 1.0, 1.0, 1.0);\n"
    //                                          "}                                    \n"};

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
    }

    ogl_state->program_handle = glCreateProgram();
    glAttachShader(ogl_state->program_handle, vertex_shader);
    glAttachShader(ogl_state->program_handle, fragment_shader);

    glLinkProgram(ogl_state->program_handle);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Vertex Array Object Creation
    glCreateVertexArrays(ogl_state->vao_len, &ogl_state->vao_handle);
    glBindVertexArray(ogl_state->vao_handle);

    return 0;
}

internal void WinPlatOpenGLPipelineDelete(WinPlatGLPipelineState* ogl_state) {
    glDeleteProgram(ogl_state->program_handle);
    glDeleteVertexArrays(ogl_state->vao_len, &ogl_state->vao_handle);
}
