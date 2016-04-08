// TODO: надо ли?
// #define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
#include <string>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <GLFW/glfw3.h>     // Непосредственно сам GLFW
#include <glm.hpp>          // библиотека графической математики
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "PngLoader.h"
#include "Helpers.h"
#include "Vertex.h"
#include "Figures.h"
#include "Shaders.h"

// Документация
// https://www.opengl.org/sdk/docs/man/html/

using namespace std;
using namespace glm;

// Текущие переменные для модели
bool enableAutoRotate = true;
vec3 modelPos = vec3(0.0f, 0.0f, -20.0f);
float xAngle = 0.0;
float yAngle = 0.0;
float zAngle = 0.0;
float size = 1.0;
bool leftButtonPressed = false;
bool rightPressed = false;
double lastCursorPosX = 0.0;
double lastCursorPosY = 0.0;

#define MATH_PI 3.14159265

void glfwErrorCallback(int error, const char* description) {
    printf("OpenGL error = %d\n description = %s\n\n", error, description);
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Выходим по нажатию Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    // по пробелу включаем или выключаем вращение автоматом
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        enableAutoRotate = !enableAutoRotate;
    }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int state, int mod) {
    // обработка левой кнопки
    if(button == GLFW_MOUSE_BUTTON_1){
        if(state == GLFW_PRESS){
            leftButtonPressed = true;
        }else{
            leftButtonPressed = false;
        }
    }
    // обработка правой кнопки
    if(button == GLFW_MOUSE_BUTTON_2){
        if(state == GLFW_PRESS){
            rightPressed = true;
        }else{
            rightPressed = false;
        }
    }
}

void glfwCursorCallback(GLFWwindow* window, double x, double y) {
    // при нажатой левой кнопки - вращаем по X и Y
    if(leftButtonPressed){
        xAngle += (y - lastCursorPosY) * 0.5;
        yAngle += (x - lastCursorPosX) * 0.5;
        // ограничение вращения
        if (xAngle < -80) {
           xAngle = -80;
        }
        if (xAngle > 80) {
           xAngle = 80;
        }
    }

    // при нажатой левой кнопки - перемещаем по X Y
    if(rightPressed){
        float offsetY = (y - lastCursorPosY) * 0.02;
        float offsetX = (x - lastCursorPosX) * 0.02;
        float newX = modelPos.x + offsetX;
        float newY = modelPos.y - offsetY;
        if (newX < -3){
            newX = -3;
        }
        if (newX > 3){
            newX = 3;
        }
        if (newY < -3){
            newY = -3;
        }
        if (newY > 3){
            newY = 3;
        }
        modelPos = vec3(newX, newY, modelPos.z);
    }

    lastCursorPosX = x;
    lastCursorPosY = y;
}

void glfwScrollCallback(GLFWwindow* window, double scrollByX, double scrollByY) {
    size += scrollByY * 0.2;
    if(size < 0.5){
        size = 0.5;
    }
    if(size > 5.0){
        size = 5.0;
    }
}

int main(int argc, char *argv[]) {

    // окно
    GLFWwindow* window = 0;

    // обработчик ошибок
    glfwSetErrorCallback(glfwErrorCallback);

    // инициализация GLFW
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }

    // создание окна
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);        // вертикальная синхронизация

    // Обработка клавиш и прочего
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(window, glfwCursorCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);

    // инициализация расширений
    glewExperimental = GL_TRUE;
    glewInit();

    // Инициализация отладки
    if(glDebugMessageCallback){
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROC)glDebugOut, 0);
        // Более высокий уровень отладки
        // GLuint unusedIds = 0;
        // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
    }

    const unsigned char* version = glGetString(GL_VERSION);
    printf("OpenGL version = %s\n", version);

    // оотношение сторон
    int width = 0;
    int height = 0;
    // Размер буффера кадра
    glfwGetFramebufferSize(window, &width, &height);
    // задаем отображение
    glViewport(0, 0, width, height);
    CHECK_GL_ERRORS();

    // Шейдеры
    GLuint shaderProgram = createShader();
    CHECK_GL_ERRORS();

    // аттрибуты вершин шейдера
    int posAttribLocation = glGetAttribLocation(shaderProgram, "aPos");
    int normalAttribLocation = glGetAttribLocation(shaderProgram, "aNormal");
    int colorAttribLocation = glGetAttribLocation(shaderProgram, "aColor");
    int aTexCoordAttribLocation = glGetAttribLocation(shaderProgram, "aTexCoord");
    CHECK_GL_ERRORS();

    // юниформы шейдера
    int modelViewProjMatrixLocation = glGetUniformLocation(shaderProgram, "uModelViewProjMat");
    int modelViewMatrixLocation = glGetUniformLocation(shaderProgram, "uModelViewMat");
    int normalMatrixLocation = glGetUniformLocation(shaderProgram, "uNormalMat");
    int lightPosViewSpaceLocation = glGetUniformLocation(shaderProgram, "uLightPosViewSpace");
    int texture1Location = glGetUniformLocation(shaderProgram, "uTexture1");
    CHECK_GL_ERRORS();

    // данные о вершинах
    GLuint VBO = 0;
    glGenBuffers (1, &VBO);
    glBindBuffer (GL_ARRAY_BUFFER, VBO);
    glBufferData (GL_ARRAY_BUFFER, piramideVertexCount * sizeof(Vertex), piramideVertexes, GL_STATIC_DRAW);
    CHECK_GL_ERRORS();

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
    CHECK_GL_ERRORS();

    // позиция света в мировых координатах
    vec3 lightPosWorldSpace = vec3(0.0, 0.0, 5.0);

    // вид
    mat4 viewMatrix = lookAt(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0));
    // Матрица проекции
    float ratio = float(width) / float(height);
    mat4 projectionMatrix = perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);

    // отключаем отображение задней части полигонов
    glEnable(GL_CULL_FACE);
    // отбрасываться будут задние грани
    glCullFace(GL_BACK);
    // Определяем, в каком направлении должный обходиться вершины, для передней части (против часовой стрелки?)
    // задняя часть будет отбрасываться
    glFrontFace(GL_CCW);
    CHECK_GL_ERRORS();

    // проверка глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    CHECK_GL_ERRORS();

    // текущее время
    double time = glfwGetTime();


    ImageData info = loadPngImage("res/test.png");
    uint textureId = 0;
    if(info.loaded){
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,              // формат внутри OpenGL
                     info.width, info.height, 0,            // ширинна, высота, границы
                     GL_RGBA, GL_UNSIGNED_BYTE, info.data); // формат входных данных
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECK_GL_ERRORS();
    }

    while (!glfwWindowShouldClose(window)){
        // приращение времени
        double newTime = glfwGetTime();
        double timeDelta = newTime - time;
        time = newTime;

        // wipe the drawing surface clear
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram (shaderProgram);

        // Вращаем на 30ть градусов автоматом
        if(enableAutoRotate){
            yAngle += timeDelta * 30.0f;
        }
        mat4 modelMatrix = mat4(1.0);
        // Здесь тоже обратный порядок умножения матриц
        // перенос
        modelMatrix = translate(modelMatrix, modelPos);
        // скейл
        modelMatrix = scale(modelMatrix, vec3(size));
        // вращение относительно осей модели
        modelMatrix = rotate(modelMatrix, float(xAngle/180.0*MATH_PI), vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = rotate(modelMatrix, float(yAngle/180.0*MATH_PI), vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = rotate(modelMatrix, float(zAngle/180.0*MATH_PI), vec3(0.0f, 0.0f, 1.0f));

        // матрица модели-камеры
        mat4 modelViewMatrix = viewMatrix * modelMatrix;

        // TODO: Вроде бы можно просто матрицу модели?
        // матрица трансформации нормалей
        mat4 normalMatrix = transpose(inverse(modelViewMatrix));

        // матрица модель-вид-проекция
        mat4 modelViewProjMatrix = projectionMatrix * viewMatrix * modelMatrix;

        // позиция света в координатах камеры
        vec3 lightPosViewSpace = vec3(viewMatrix * vec4(lightPosWorldSpace, 1.0));

        // выставляем матрицу трансформа в координаты камеры
        glUniformMatrix4fv(modelViewMatrixLocation, 1, false, glm::value_ptr(modelMatrix));
        // выставляем матрицу трансформа нормалей
        glUniformMatrix4fv(normalMatrixLocation, 1, false, glm::value_ptr(normalMatrix));
        // выставляем матрицу трансформации в пространство OpenGL
        glUniformMatrix4fv(modelViewProjMatrixLocation, 1, false, glm::value_ptr(modelViewProjMatrix));
        // выставляем позицию света в координатах камеры
        glUniform3f(lightPosViewSpaceLocation, lightPosViewSpace.x, lightPosViewSpace.y, lightPosViewSpace.z);
        // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
        glUniform1i(texture1Location, 0);

        // активируем нулевую текстуру для для шейдера, включаем эту текстуру
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // рисуем
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, piramideVertexCount); // draw points 0-3 from the currently bound VAO with current in-use shader
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
