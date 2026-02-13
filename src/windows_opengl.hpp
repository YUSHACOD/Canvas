#ifndef WIN_OPENGL_H
#define WIN_OPENGL_H
// Opengl macros, function globals, and structs ------------------------------------------------ //


#include <windows.h>
#include <GL\gl.h>

#include "base/sugars.hpp"

typedef char GLchar;

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31
#define GL_COMPILE_STATUS  0x8B81

typedef BOOL   wgl_swap_interval_ext(int interval);
typedef GLuint gl_create_shader(GLenum type);
typedef void
gl_shader_source(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void   gl_compile_shader(GLuint shader);
typedef GLuint gl_create_program(void);
typedef void   gl_attach_shader(GLuint program, GLuint shader);
typedef void   gl_link_program(GLuint program);
typedef void   gl_delete_shader(GLuint shader);
typedef void   gl_delete_program(GLuint program);
typedef void   gl_create_vertex_arrays(GLsizei n, GLuint* arrays);
typedef void   gl_bind_vertex_array(GLuint array);
typedef void   gl_delete_vertex_arrays(GLsizei n, const GLuint* arrays);
typedef void   gl_use_program(GLuint program);
typedef void   gl_get_shaderiv(GLuint shader, GLenum pname, GLint* params);
typedef void
gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void gl_clear_bufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value);
typedef void gl_vertex_attrib4fv(GLuint index, const GLfloat* v);
typedef void gl_vertex_attrib1f(GLuint index, const GLfloat x);
typedef void
gl_uniform_matrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef GLint gl_get_uniform_location(GLuint program, const GLchar* name);

global wgl_swap_interval_ext*   wglSwapIntervalEXT;
global gl_create_shader*        glCreateShader;
global gl_shader_source*        glShaderSource;
global gl_compile_shader*       glCompileShader;
global gl_create_program*       glCreateProgram;
global gl_attach_shader*        glAttachShader;
global gl_link_program*         glLinkProgram;
global gl_delete_shader*        glDeleteShader;
global gl_delete_program*       glDeleteProgram;
global gl_create_vertex_arrays* glCreateVertexArrays;
global gl_bind_vertex_array*    glBindVertexArray;
global gl_delete_vertex_arrays* glDeleteVertexArrays;
global gl_use_program*          glUseProgram;
global gl_get_shaderiv*         glGetShaderiv;
global gl_get_shader_info_log*  glGetShaderInfoLog;
global gl_clear_bufferfv*       glClearBufferfv;
global gl_vertex_attrib4fv*     glVertexAttrib4fv;
global gl_vertex_attrib1f*      glVertexAttrib1f;
global gl_uniform_matrix4fv*    glUniformMatrix4fv;
global gl_get_uniform_location* glGetUniformLocation;

typedef struct {
    GLuint vao_len;
    GLuint vao_handle;
    GLuint program_handle;
    GLuint projection_location;
} GLPipelineState;
// --------------------------------------------------------------------------------------------- //
#endif
