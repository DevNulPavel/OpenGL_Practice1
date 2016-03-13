#ifndef HELPERS
#define HELPERS

#define GLFW_INCLUDE_GLU
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее

// вычисление смещения в структуре/классе
#define OFFSETOF(TYPE, FIELD) ((void*)&(((TYPE*)NULL)->FIELD))
// Превращаем текущий текст в строку шейдера
#define STRINGIFY_SHADER(TEXT) ("#version 120\n "#TEXT)
// проверка ошибок
#define CHECK_GL_ERRORS() checkOpenGLerror(__FILE__, __LINE__)

int checkOpenGLerror(const char *file, int line);
void glDebugOut(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);


#endif
