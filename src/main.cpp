// TODO: надо ли?
// #define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
//#define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#include <functional>
#include <thread>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

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

    while (!glfwWindowShouldClose(window)){

        glClear(GL_COLOR_BUFFER_BIT);

        // ТУТ рендеринг
       GLuint vs = glCreateShader(GL_VERTEX_SHADER);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
