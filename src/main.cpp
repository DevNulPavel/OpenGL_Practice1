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
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }

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
    // вертикальная синхронизация
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
        // vertex attribute
        attribute vec3 aPos;
        attribute vec3 aNormal;
        attribute vec3 aColor;
        attribute vec3 aTexCoord;
        // uniforms
        uniform mat4 uModelViewMat;
        uniform mat4 uNormalMat;
        uniform mat4 uModelViewProjMat;
        // output
        varying vec3 vPosViewSpace;
        varying vec3 vNormalViewSpace;
        varying vec3 vColor;
        varying vec3 vTexCoord;

        void main () {
            vec4 vertexVec4 = vec4(aPos, 1.0);      // последняя компонента 1, тк это точка
            vec4 normalVec4 = vec4(aNormal, 0.0);   // последняя компонента 0, тк это направление
            // вычисляем позицию точки в пространстве OpenGL
            gl_Position = uModelViewProjMat * vertexVec4;
            // вычисляем позицию и нормаль в пространстве камеры
            vPosViewSpace = vec3(uModelViewMat * vertexVec4);                   // это вершина, учитывается перенос и тд
            vNormalViewSpace = normalize(vec3(uNormalMat * normalVec4));    // так как это направление, учитывается поворот только
            // цвет и текстурные координаты просто пробрасываем для интерполяции
            vColor = aColor;
            vTexCoord = aTexCoord;
        }
    );
    const char* fragmentShader = STRINGIFY_SHADER(
        varying vec3 vPosViewSpace;
        varying vec3 vNormalViewSpace;
        varying vec3 vColor;
        varying vec3 vTexCoord;

        uniform vec3 uLightPosViewSpace;

        const float ambientCoef = 0.1;
        const float diffuseCoef = 0.5;
        const float specularCoeff = 0.1;
        const float specularShinnes = 1.0;  // >0

        void main () {
            vec3 fromTexelToLightDir = normalize(uLightPosViewSpace - vPosViewSpace);
            vec3 fromTexelToEyesDir = normalize(-vPosViewSpace);
            vec3 texelLightReflectionDir = normalize(reflect(-fromTexelToLightDir, vNormalViewSpace));

            // Диффузное
            // на сколкьо сильно совпадает направление нормали и направления к свету
            float diffusePower = diffuseCoef * max(dot(vNormalViewSpace, fromTexelToLightDir), 0.0);

            // Блики
            // Степень того, как сильно совпадает направление отражения света и направление в камеру
            float specularDot = max(dot(texelLightReflectionDir, fromTexelToEyesDir), 0.0);
            float specularPower = 0.0;
            // проверка, так как 0 в любой степени - это 1.0
            if(specularDot > 0.0){
               specularPower = pow(specularDot, specularShinnes);
               specularPower = clamp(specularPower, 0.0, 1.0);
            }

            float lightPower = ambientCoef + diffusePower + specularPower;

            gl_FragColor = vec4(lightPower, lightPower, lightPower, 1.0);
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
    int modelViewMatrixLocation = glGetUniformLocation(shaderProgram, "uModelViewMat");
    int normalMatrixLocation = glGetUniformLocation(shaderProgram, "uNormalMat");
    int lightPosViewSpaceLocation = glGetUniformLocation(shaderProgram, "uLightPosViewSpace");

    // данные о вершинах
    int vertexCount = 18;
    Vertex points[] = {
        //              ВЕРШИНА                     НОРМАЛЬ                 ЦВЕТ                ТЕКСТУРНЫЕ_КООРДИНАТЫ
        // первая грань, передняя
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f)),
        // вторая, правая
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f)),
        // третья, задняя
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f)),
        // четвертая, левая
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f)),
        // пятая, низ пирамиды 1
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f)),
        // шестая, низ пирамиды 2
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(0.5f, 0.0f, 0.5f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(0.0f, 0.5f, 0.5f), vec2(0.0f, 0.0f))
    };
    GLuint VBO = 0;
    glGenBuffers (1, &VBO);
    glBindBuffer (GL_ARRAY_BUFFER, VBO);
    glBufferData (GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), points, GL_STATIC_DRAW);
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

    // проверка глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float angle = 0.0;
    // текущее время
    double time = glfwGetTime();

    while (!glfwWindowShouldClose(window)){
        // приращение времени
        double newTime = glfwGetTime();
        double timeDelta = newTime - time;
        time = newTime;

        // wipe the drawing surface clear
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram (shaderProgram);

        // Вращаем на 5ть градусов
        angle += timeDelta * 5.0f;
        mat4 modelMatrix = mat4(1.0);
        // Здесь тоже обратный порядок умножения матриц
        modelMatrix = translate(modelMatrix, vec3(0.0f, 0.0f, -10.0f));
        modelMatrix = rotate(modelMatrix, float(angle/M_PI/2.0), vec3(1.0f, 1.0f, 0.0f));

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

        // рисуем
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount); // draw points 0-3 from the currently bound VAO with current in-use shader
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
