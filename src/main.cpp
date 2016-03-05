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
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

// Документация
// https://www.opengl.org/sdk/docs/man/html/

using namespace glm;

// вычисление смещения в структуре/классе
#define OFFSETOF(TYPE, FIELD) ((void*)&(((TYPE*)NULL)->FIELD))
// Превращаем текущий текст в строку шейдера
#define STRINGIFY_SHADER(TEXT) ("#version 120\n "#TEXT)

struct Vertex{
    vec3 pos;
    vec3 normal;
    vec3 color;
    vec2 texCoord;

    // constructor
    Vertex(vec3 inPos, vec3 inNormal, vec3 inColor, vec2 inTexCoord):
        pos(inPos),
        normal(inNormal),
        color(inColor),
        texCoord(inTexCoord){
    }
};

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
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
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
    int width = 0;
    int height = 0;
    // Размер буффера кадра
    glfwGetFramebufferSize(window, &width, &height);
    // задаем отображение
    glViewport(0, 0, width, height);


    // Шейдеры
    const char* vertexShader = STRINGIFY_SHADER(
        attribute vec3 aPos;
        attribute vec3 aNormal;
        attribute vec3 aColor;
        attribute vec3 aTexCoord;

        uniform mat4 uModelViewProjMat;

        varying vec3 vColor;
        varying vec3 vTexCoord;

        void main () {
            gl_Position = uModelViewProjMat * vec4(aPos, 1.0);
            vColor = aColor;
            vTexCoord = aTexCoord;
        }
    );
    const char* fragmentShader = STRINGIFY_SHADER(
        varying vec3 vColor;
        varying vec3 vTexCoord;

        void main () {
            gl_FragColor = vec4(vColor, 1.0);
        }
    );
    
    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vs, 1, &vertexShader, NULL);
    glCompileShader (vs);
    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fs, 1, &fragmentShader, NULL);
    glCompileShader (fs);
    checkOpenGLerror();
    
    GLuint shaderProgram = glCreateProgram ();
    glAttachShader (shaderProgram, fs);
    glAttachShader (shaderProgram, vs);
    glLinkProgram (shaderProgram);
    checkOpenGLerror();
    
    // аттрибуты вершин шейдера
    int posAttribLocation = glGetAttribLocation(shaderProgram, "aPos");
    int normalAttribLocation = glGetAttribLocation(shaderProgram, "aNormal");
    int colorAttribLocation = glGetAttribLocation(shaderProgram, "aColor");
    int aTexCoordAttribLocation = glGetAttribLocation(shaderProgram, "aTexCoord");

    // юниформы шейдера
    int modelViewProjMatrixLocation = glGetUniformLocation(shaderProgram, "uModelViewProjMat");

    // данные о вершинах
    Vertex points[] = {
        Vertex(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3(1.0f, -1.0f,  0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f))
    };
    GLuint VBO = 0;
    glGenBuffers (1, &VBO);
    glBindBuffer (GL_ARRAY_BUFFER, VBO);
    glBufferData (GL_ARRAY_BUFFER, 3 * sizeof(Vertex), points, GL_STATIC_DRAW);
    checkOpenGLerror();

    // VAO
    GLuint vao = 0;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    // sizeof(Vertex) - размер блока одной информации о вершине
    // OFFSETOF(Vertex, color) - смещение от начала
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Позиции
    glEnableVertexAttribArray(posAttribLocation);
    glVertexAttribPointer(posAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, pos));
    // Нормали
    glEnableVertexAttribArray(normalAttribLocation);
    glVertexAttribPointer(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, normal));
    // Цвет вершин
    glEnableVertexAttribArray(colorAttribLocation);
    glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, color));
    // Текстурные координаты
    glEnableVertexAttribArray(aTexCoordAttribLocation);
    glVertexAttribPointer(aTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, texCoord));
    // off
    glBindVertexArray(0);
    checkOpenGLerror();

    // вид
    mat4 viewMatrix = lookAt(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0));
    // Матрица проекции
    float ratio = float(width) / float(height);
    mat4 projectionMatrix = perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    float angle = 0.0;

    // отключаем отображение задней части полигонов
    glEnable(GL_CULL_FACE);
    // отбрасываться будут задние грани
    glCullFace(GL_BACK);
    // Определяем, в каком направлении должный обходиться вершины, для передней части (против часовой стрелки?)
    // задняя часть будет отбрасываться
    glFrontFace(GL_CCW);

    while (!glfwWindowShouldClose(window)){

        // wipe the drawing surface clear
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram (shaderProgram);

        // Вращаем
        angle += 0.05;
        mat4 modelMatrix = mat4(1.0);
        // Здесь тоже обратный порядок умножения матриц
        modelMatrix = translate(modelMatrix, vec3(0.0f, 0.0f, -10.0f));
        modelMatrix = rotate(modelMatrix, float(angle/M_PI), vec3(0.0f, 1.0f, 0.0f));

        // выставляем матрицу трансформации
        mat4 modelViewProjMatrix = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(modelViewProjMatrixLocation, 1, false, glm::value_ptr(modelViewProjMatrix));

        // рисуем
        glBindVertexArray(vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

//    glDeleteProgram(shaderProgram);
//    glDeleteBuffers(1, &vbo);
//    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
