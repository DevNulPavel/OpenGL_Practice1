// TODO: надо ли?
// #define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
#include <vector>
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
#include "ObjLoader.h"
#include "RenderManager.h"
#include "UIManager.h"
#include "PostProcessManager.h"

#define MATH_PI 3.14159265


// Документация
// https://www.opengl.org/sdk/docs/man/html/

using namespace std;
using namespace glm;

// Текущие переменные для модели
bool leftButtonPressed = false;
bool rightPressed = false;
double lastCursorPosX = 0.0;
double lastCursorPosY = 0.0;

RenderManager* render = nullptr;
UIManager* uiManager = nullptr;
PostProcessManager* postProcessManager = nullptr;


///////////////////////////////////////////////////////////////////////////////////////////


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
        if (render) {
            render->setEnableAutoRotate(!render->getEnableAutoRotate());
        }
    }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int state, int mod) {
    // обработка левой кнопки
    if(button == GLFW_MOUSE_BUTTON_1){
        if(state == GLFW_PRESS){
            leftButtonPressed = true;
        }else{
            if (uiManager && (leftButtonPressed == true)) {
                uiManager->tappedOnPos(vec2(lastCursorPosX, lastCursorPosY));
            }
        
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
        float xAngle = 0.0;
        float yAngle = 0.0;
        if (render) {
            xAngle = render->getXAngle();
            yAngle = render->getYAngle();
        }
    
        xAngle += (y - lastCursorPosY) * 0.5;
        yAngle += (x - lastCursorPosX) * 0.5;
        // ограничение вращения
        if (xAngle < -80) {
           xAngle = -80;
        }
        if (xAngle > 80) {
           xAngle = 80;
        }
        
        if (render) {
            render->setXAngle(xAngle);
            render->setYAngle(yAngle);
        }
    }

    // при нажатой левой кнопки - перемещаем по X Y
    if(rightPressed){
        vec3 modelPos;
        if (render) {
            modelPos = render->getModelPos();
        }
    
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
        
        if (render) {
            render->setModelPos(modelPos);
        }
    }

    lastCursorPosX = x;
    lastCursorPosY = y;
}

void glfwScrollCallback(GLFWwindow* window, double scrollByX, double scrollByY) {
    float size = 0;
    if (render) {
        size = render->getSize();
    }

    size += scrollByY * 0.2;
    if(size < 0.5){
        size = 0.5;
    }
    if(size > 5.0){
        size = 5.0;
    }
    
    if (render) {
        render->setSize(size);
    }
}

void setupOpenGL(GLFWwindow*& window){
    // окно
    window = 0;
    
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
        
        // Коллбек ошибок OpenGL
        glDebugMessageCallback((GLDEBUGPROC)glDebugOut, 0);
        
        // Более высокий уровень отладки
        // GLuint unusedIds = 0;
        // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
    }
    
    const unsigned char* version = glGetString(GL_VERSION);
    printf("OpenGL version = %s\n", version);
}

int main(int argc, char *argv[]) {
    GLFWwindow* window = 0;
    setupOpenGL(window);

    // оотношение сторон
    int width = 0;
    int height = 0;
    // Размер буффера кадра
    glfwGetFramebufferSize(window, &width, &height);
    
    // задаем отображение
    glViewport(0, 0, width, height);
    CHECK_GL_ERRORS();

    // текущее время
    double time = glfwGetTime();

    // рендер 3д графики
    render = new RenderManager(width, height);
    
    // графический интерфейс
    uiManager = new UIManager(width, height);
    // элемент 1
    UIElementPtr element = make_shared<UIElement>("res/button_green_normal.png");
    element->setPosition(vec2(100));
    element->setCallback([](){ cout << "Button pressed" << endl; });
    uiManager->addElement(element);
    
    // менеджер постпроцессинга
    postProcessManager = new PostProcessManager(width, height);
    
    
    while (!glfwWindowShouldClose(window)){
        // приращение времени
        double newTime = glfwGetTime();
        double timeDelta = newTime - time;
        time = newTime;

        // wipe the drawing surface clear
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rendering
        postProcessManager->grabStart();
        if (render) {
            render->draw(timeDelta);
        }
        if (uiManager) {
            uiManager->draw(timeDelta);
        }
        postProcessManager->grabEnd();
        
        // PostProc draw
        postProcessManager->draw(timeDelta);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete render;
    delete uiManager;
    delete postProcessManager;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}

//! [code]
