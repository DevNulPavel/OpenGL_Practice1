// TODO: надо ли?
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
// #define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#include <functional>
#include <thread>
#include <stdio.h>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <GLFW/glfw3.h>     // Непосредственно сам GLFW
#include <glm.hpp>          // библиотека графической математики


// Документация
// https://www.opengl.org/sdk/docs/man/html/


static void error_callback(int error, const char* description) {
    printf("OpenGL error = %d\n description = %s\n\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void checkOpenGLerror() {
    GLenum errCode = GL_NO_ERROR;
    if((errCode=glGetError()) != GL_NO_ERROR){
        printf("OpenGl error! %d - %s\n", errCode, glewGetErrorString(errCode));
    }
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
    
    // инициализация расширений
    glewExperimental = GL_TRUE;
    glewInit();

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

    
    const char* vertex_shader =
    "#version 120\n"
    "attribute vec3 aPos;"
    "attribute vec3 aColor;"
    "varying vec3 vColor;"
    "void main () {"
    "   gl_Position = vec4(aPos, 1.0);"
    "   vColor = aColor;"
    "}";
    
    const char* fragment_shader =
    "#version 120\n"
    "varying vec3 vColor;"
    "void main () {"
    "  gl_FragColor = vec4(vColor, 1.0);"
    "}";
    
    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vs, 1, &vertex_shader, NULL);
    glCompileShader (vs);
    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fs, 1, &fragment_shader, NULL);
    glCompileShader (fs);
    checkOpenGLerror();
    
    GLuint shader_program = glCreateProgram ();
    glAttachShader (shader_program, fs);
    glAttachShader (shader_program, vs);
    glLinkProgram (shader_program);
    checkOpenGLerror();
    
    // аттрибуты шейдера
    int posAttrib = glGetAttribLocation(shader_program, "aPos");
    int colorAttrib = glGetAttribLocation(shader_program, "aColor");

    // данные о вершинах
    float points[] = {
       0.0f,  0.5f,  0.0f,
       -0.5f, 0.5f,  0.0f,
       -0.5f, -0.5f,  0.0f
    };
    GLuint posVBO = 0;
    glGenBuffers (1, &posVBO);
    glBindBuffer (GL_ARRAY_BUFFER, posVBO);
    glBufferData (GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
    checkOpenGLerror();

    // данные о цветах
    float colors[] = {
       0.0f,  0.5f,  0.0f,
       0.5f, 0.5f,  0.0f,
       0.5f, 0.5f,  1.0f
    };
    GLuint colorVBO = 0;
    glGenBuffers (1, &colorVBO);
    glBindBuffer (GL_ARRAY_BUFFER, colorVBO);
    glBufferData (GL_ARRAY_BUFFER, 9 * sizeof(float), colors, GL_STATIC_DRAW);
    checkOpenGLerror();

    // VAO
    GLuint vao = 0;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    // Позиции
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    // Цвет вершин
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    // off
    glBindVertexArray(0);
    checkOpenGLerror();

    while (!glfwWindowShouldClose(window)){

        // wipe the drawing surface clear
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram (shader_program);
        
//        // ! Включаем массив атрибутов
//        const char* attr_name = "vp";
//        int attrib = glGetAttribLocation(shader_program, attr_name);
//        glEnableVertexAttribArray(attrib);
//        // ! Подключаем VBO
//        glBindBuffer(GL_ARRAY_BUFFER, vbo);
//        // ! Указывая pointer 0 при подключенном буфере, мы указываем, что данные представлены в VBO
//        glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
//        // ! Передаем данные на видеокарту (рисуем)
//        glDrawArrays(GL_TRIANGLES, 0, 3);
//        // ! Отключаем VBO
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        
//        // ! Отключаем массив атрибутов
//        glDisableVertexAttribArray(attrib);

        
        glBindVertexArray(vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        checkOpenGLerror();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

//    glDeleteProgram(shader_program);
//    glDeleteBuffers(1, &vbo);
//    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
