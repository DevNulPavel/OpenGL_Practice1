// TODO: надо ли?
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
//#define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#include <functional>
#include <thread>
#include <stdio.h>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <GLFW/glfw3.h>     // Непосредственно сам GLFW
#include <glm.hpp>          // библиотека графической математики

static void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void) {

    // окно
    GLFWwindow* window = 0;

    // обработчик ошибок
    glfwSetErrorCallback(error_callback);

    // инициализация GLFW
    if (!glfwInit())
        exit(EXIT_FAILURE);

    // создание окна
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    const unsigned char* version = glGetString(GL_VERSION);
    printf("OpenGL version = %s\n", version);

    // Обработка клавиш
    glfwSetKeyCallback(window, key_callback);

    // оотношение сторон
    float ratio = 1.0;
    int width = 0;
    int height = 0;

    // получаем соотношение сторон
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;

    // задаем отображение
    glViewport(0, 0, width, height);

    float points[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, -0.5f,  0.0f,
      -0.5f, -0.5f,  0.0f
    };

//    GLuint vbo = 0;
//    glGenBuffers (1, &vbo);
//    glBindBuffer (GL_ARRAY_BUFFER, vbo);
//    glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), points, GL_STATIC_DRAW);

//    GLuint vao = 0;
//    glGenVertexArrays (1, &vao);
//    glBindVertexArray (vao);
//    glEnableVertexAttribArray (0);
//    glBindBuffer (GL_ARRAY_BUFFER, vbo);
//    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

//    const char* vertex_shader =
//    "#version 120\n"
//    "in vec3 vp;"
//    "void main () {"
//    "   gl_Position = vec4 (vp, 1.0);"
//    "}";

//    const char* fragment_shader =
//    "#version 120\n"
//    "out vec4 frag_colour;"
//    "void main () {"
//    "  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
//    "}";

//    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
//    glShaderSource (vs, 1, &vertex_shader, NULL);
//    glCompileShader (vs);
//    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
//    glShaderSource (fs, 1, &fragment_shader, NULL);
//    glCompileShader (fs);

//    GLuint shader_program = glCreateProgram ();
//    glAttachShader (shader_program, fs);
//    glAttachShader (shader_program, vs);
//    glLinkProgram (shader_program);

    while (!glfwWindowShouldClose(window)){

        // wipe the drawing surface clear
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glUseProgram (shader_program);
//        glBindVertexArray (vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
//        glDrawArrays (GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
