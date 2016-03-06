// TODO: надо ли?
#define GLFW_INCLUDE_GLU
#define GLFW_INCLUDE_GL3
#define GLFW_INCLUDE_GLEXT
// #define GLFW_INCLUDE_GLCOREARB 1 // Tell GLFW to include the OpenGL core profile header
#include <string>
#include <stdio.h>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <GLFW/glfw3.h>     // Непосредственно сам GLFW
#include <glm.hpp>          // библиотека графической математики
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <png.h>

// Документация
// https://www.opengl.org/sdk/docs/man/html/

using namespace std;
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

struct ImageData{
    bool loaded;
    size_t dataSize;
    char* data;
    int width;
    int height;
    // constructor
    ImageData(size_t inDataSize, char* inData, int inWidth, int inHeight){
        width = inWidth;
        height = inHeight;
        if(inDataSize > 0){
            data = inData;
            loaded = true;
        }else{
            data = NULL;
            loaded = false;
        }
    }
    // destructor
    ~ImageData(){
        if(data != NULL){
            delete data;
        }
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

ImageData loadPngImage(const char* fileName){
    // проверяем сигнатуру файла (первые 4 байт)
    size_t headerSize = 8;
    png_byte header[headerSize];
    memset(header, 0, headerSize);
    FILE* fp = fopen(fileName, "rb");
    if(fp == NULL){
        printf("File %s nor found\n", fileName);
        return ImageData(0, NULL, 0, 0);
    }

    fread(header, 1, headerSize, fp);
    if (png_check_sig(header, headerSize) == false) {
        fclose(fp);
        printf("Is not png: %s\n", fileName);
        return ImageData(0, NULL, 0, 0);
    }
    
    // создаем внутреннюю структуру png для работы с файлом
    // последние параметры - структура, для функции обработки ошибок и варнинга (последн. 2 параметра)
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    // создаем структуру с информацией о файле
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    //> Save calling environment for long jump
    setjmp(png_jmpbuf(png_ptr));

    //> Initialize input/output for the PNG file
    png_init_io(png_ptr, fp);

    // говорим библиотеке, что мы уже прочли 4 байт, когда проверяли сигнатуру
    png_set_sig_bytes(png_ptr, headerSize);
    // читаем всю информацию о файле
    png_read_info(png_ptr, info_ptr);
    // Эта функция возвращает инфу из info_ptr
    // размер картинки в пикселях
    png_uint_32 width = 0;
    png_uint_32 height = 0;
    // глубина цвета (одного из каналов, может быть 1, 2, 4, 8, 16)
    int bit_depth = 0;
    // описывает какие каналы присутствуют:
    // PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE,
    // PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA...
    int color_type = 0;
    // последние 3 параметра могут быть нулями и обозначают: тип фильтра, тип компрессии и тип смещения
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);

    // png формат может содержать 16 бит на канал, но нам нужно только 8, поэтому сужаем канал
    if (bit_depth == 16) png_set_strip_16(png_ptr);
    // преобразуем файл если он содержит палитру в нормальный RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) png_set_palette_to_rgb(png_ptr);
    // если в грэйскейле меньше бит на канал чем 8, то конвертим к нормальному 8-битному
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
    // и добавляем полный альфа-канал
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    // В нашей игре допустимы картинки, содержащие информацию только об оттенках серого цвета (grayscale картинки).
    // Если же, необходимо преобразование к RGB формату,
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);

    double gamma = 0.0f;
    // если есть информация о гамме в файле, то устанавливаем на 2.2
    if ( png_get_gAMA(png_ptr, info_ptr, &gamma) ) png_set_gamma(png_ptr, 2.2, gamma);
    // иначе ставим дефолтную гамму для файла в 0.45455 (good guess for GIF images on PCs)
    else png_set_gamma(png_ptr, 2.2, 0.45455);

    // после всех трансформаций, апдейтим информацию в библиотеке
    png_read_update_info(png_ptr, info_ptr);
    // опять получаем все размеры и параметры обновленной картинки
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);

    // определяем кол-во байт нужных для того чтобы вместить строку
    png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    // теперь, мы можем выделить память чтобы вместить картинку
    size_t dataSize = row_bytes * height;
    png_byte* data = new png_byte[dataSize];
    memset(data, 0, dataSize);
    // выделяем память, для указателей на каждую строку
    png_byte **row_pointers = new png_byte * [height];
    // сопоставляем массив указателей на строчки, с выделенными в памяти (res)
    // т.к. изображение перевернутое, то указатели идут снизу вверх
    for (unsigned int i = 0; i < height; i++){
        row_pointers[height - i - 1] = data + i * row_bytes;
    }

    // все, читаем картинку (данные)
    png_read_image(png_ptr, row_pointers);

    // освобождаем память от указателей на строки
    delete [] row_pointers;

    // освобождаем память выделенную для библиотеки libpng
    png_destroy_read_struct(&png_ptr, 0, 0);

    // закрываем файл
    fclose(fp);

    // TODO: формат
    return ImageData(dataSize, (char*)data, width, height);
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
        attribute vec2 aTexCoord;
        // uniforms
        uniform mat4 uModelViewMat;
        uniform mat4 uNormalMat;
        uniform mat4 uModelViewProjMat;
        // output
        varying vec3 vPosViewSpace;
        varying vec3 vNormalViewSpace;
        varying vec3 vColor;
        varying vec2 vTexCoord;

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
        varying vec2 vTexCoord;

        uniform sampler2D uTexture1;
        uniform vec3 uLightPosViewSpace;

        const float ambientCoef = 0.5;
        const float diffuseCoef = 0.5;
        const float specularCoeff = 1.0;
        const float specularShinnes = 3.0;  // >0

        void main () {
            // Рассчет освещения
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

            // текстура
            vec3 textureColor = vec3(texture2D(uTexture1, vTexCoord));

            gl_FragColor = vec4(textureColor * vColor * lightPower, 1.0);
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
    int texture1Location = glGetUniformLocation(shaderProgram, "uTexture1");

    // данные о вершинах
    int vertexCount = 18;
    Vertex points[] = {
        //              ВЕРШИНА                     НОРМАЛЬ                 ЦВЕТ                ТЕКСТУРНЫЕ_КООРДИНАТЫ
        // первая грань, передняя
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
        // вторая, правая
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
        // третья, задняя
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
        // четвертая, левая
        Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
        // пятая, низ пирамиды 1
        Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 1.0f)),
        // шестая, низ пирамиды 2
        Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 1.0f)),
        Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 1.0f))
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


    ImageData info = loadPngImage("/home/devnul/Projects/OpenGL_Practice1/test.png");
    uint textureId = 0;
    if(info.loaded){
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,              // формат внутри OpenGL
                     info.width, info.height, 0,            // ширинна, высота, границы
                     GL_RGBA, GL_UNSIGNED_BYTE, info.data); // формат входных данных
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
        // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
        glUniform1i(texture1Location, 0);

        // активируем нулевую текстуру для для шейдера, включаем эту текстуру
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);

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
