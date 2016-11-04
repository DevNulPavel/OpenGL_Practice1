#include "RenderManager.h"
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shaders.h"
#include "Vertex.h"
#include "ObjLoader.h"
#include "PngLoader.h"

#define MATH_PI 3.14159265

// расположение аттрибутов (с 0)
#define POS_ATTRIB_LOCATION 0
#define NORMAL_ATTRIB_LOCATION 1
#define COLOR_ATTRIB_LOCATION 2
#define TEX_COORD_ATTRIB_LOCATION 3

RenderManager::RenderManager(float width, float height){
    _enableAutoRotate = true;
    _modelPos = vec3(0.0f, 0.0f, -20.0f);
    _xAngle = 0.0;
    _yAngle = 0.0;
    _zAngle = 0.0;
    _size = 1.0;

    // изменение размера
    resize(width, height);
    
    // Шейдеры
    map<string, int> attributesLocations;
    attributesLocations["aPos"] = POS_ATTRIB_LOCATION;
    attributesLocations["aNormal"] = NORMAL_ATTRIB_LOCATION;
    attributesLocations["aColor"] = COLOR_ATTRIB_LOCATION;
    attributesLocations["aTexCoord"] = TEX_COORD_ATTRIB_LOCATION;
    _shaderProgram = create3DShader(attributesLocations);
    CHECK_GL_ERRORS();
    
    // юниформы шейдера
    _modelViewProjMatrixLocation = glGetUniformLocation(_shaderProgram, "uModelViewProjMat");
    _modelViewMatrixLocation = glGetUniformLocation(_shaderProgram, "uModelViewMat");
    _normalMatrixLocation = glGetUniformLocation(_shaderProgram, "uNormalMat");
    _lightPosViewSpaceLocation = glGetUniformLocation(_shaderProgram, "uLightPosViewSpace");
    _texture1Location = glGetUniformLocation(_shaderProgram, "uTexture1");
    CHECK_GL_ERRORS();

    ////// выбор модели
    ///// Пирамида
    // size_t modelVertexCount = piramideVertexCount;
    // Vertex* modelVertexesData = piramideVertexes;
    ///// Какая-то модель Obj
    vector<Vertex> modelVertexes;
    loadObjModel("res/sphere.obj", modelVertexes);
    _modelVertexCount = modelVertexes.size();
    Vertex* modelVertexesData = modelVertexes.data();
    
    // VBO, данные о вершинах
    _vbo = 0;
    glGenBuffers (1, &_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, _vbo);
    glBufferData (GL_ARRAY_BUFFER, _modelVertexCount * sizeof(Vertex), (void*)modelVertexesData, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
    
    // позиция света в мировых координатах
    _lightPosWorldSpace = vec3(0.0, 0.0, 5.0);
    
    // вид
    _viewMatrix = lookAt(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0));
    
    // Загрузка текстуры
    ImageData info = loadPngImage("res/sphere.png");
    //  ImageData info = loadPngImage("res/test.png");
    _textureId = 0;
    if(info.loaded){
        glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,             // формат внутри OpenGL
                     info.width, info.height, 0,            // ширинна, высота, границы
                     info.withAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, info.data); // формат входных данных
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECK_GL_ERRORS();
    }
}

RenderManager::~RenderManager(){
    glDeleteTextures(1, &_textureId);
    glDeleteProgram(_shaderProgram);
    glDeleteBuffers(1, &_vbo);
}

void RenderManager::resize(float width, float height){
    // Матрица проекции
    float ratio = float(width) / float(height);
    _projectionMatrix = perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
}

void RenderManager::draw(float delta){
    // отключаем отображение задней части полигоновu
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
    
    // Вращаем на 30ть градусов автоматом
    if(_enableAutoRotate){
        _yAngle += delta * 30.0f;
    }
    mat4 modelMatrix = mat4(1.0);
    // Здесь тоже обратный порядок умножения матриц
    // перенос
    modelMatrix = translate(modelMatrix, _modelPos);
    // скейл
    modelMatrix = scale(modelMatrix, vec3(_size));
    // вращение относительно осей модели
    modelMatrix = rotate(modelMatrix, float(_xAngle/180.0*MATH_PI), vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = rotate(modelMatrix, float(_yAngle/180.0*MATH_PI), vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = rotate(modelMatrix, float(_zAngle/180.0*MATH_PI), vec3(0.0f, 0.0f, 1.0f));
    
    // матрица модели-камеры
    mat4 modelViewMatrix = _viewMatrix * modelMatrix;
    
    // TODO: Вроде бы можно просто матрицу модели?
    // матрица трансформации нормалей
    mat4 normalMatrix = transpose(inverse(modelViewMatrix));
    
    // матрица модель-вид-проекция
    mat4 modelViewProjMatrix = _projectionMatrix * _viewMatrix * modelMatrix;
    
    // позиция света в координатах камеры
    vec3 lightPosViewSpace = vec3(_viewMatrix * vec4(_lightPosWorldSpace, 1.0));
    
    // включение шейдера
    glUseProgram (_shaderProgram);
    CHECK_GL_ERRORS();
    
    // выставляем матрицу трансформа в координаты камеры
    glUniformMatrix4fv(_modelViewMatrixLocation, 1, false, glm::value_ptr(modelMatrix));
    // выставляем матрицу трансформа нормалей
    glUniformMatrix4fv(_normalMatrixLocation, 1, false, glm::value_ptr(normalMatrix));
    // выставляем матрицу трансформации в пространство OpenGL
    glUniformMatrix4fv(_modelViewProjMatrixLocation, 1, false, glm::value_ptr(modelViewProjMatrix));
    // выставляем позицию света в координатах камеры
    glUniform3f(_lightPosViewSpaceLocation, lightPosViewSpace.x, lightPosViewSpace.y, lightPosViewSpace.z);
    // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
    glUniform1i(_texture1Location, 0);
    CHECK_GL_ERRORS();
    
    // активируем нулевую текстуру для для шейдера, включаем эту текстуру
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    CHECK_GL_ERRORS();
    
    // рисуем
    //      sizeof(Vertex) - размер блока одной информации о вершине
    //      OFFSETOF(Vertex, color) - смещение от начала
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    
    // VBO enable attrib
    glEnableVertexAttribArray(POS_ATTRIB_LOCATION);
    glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
    glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
    glEnableVertexAttribArray(TEX_COORD_ATTRIB_LOCATION);
    
    // VBO align
    glVertexAttribPointer(POS_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, pos));   // Позиции
    glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, normal)); // Нормали
    glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, color));   // Цвет вершин
    glVertexAttribPointer(TEX_COORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSETOF(Vertex, texCoord));    // Текстурные координаты
    
    // draw
    glDrawArrays(GL_TRIANGLES, 0, _modelVertexCount); // draw points 0-3 from the currently bound VAO with current in-use shader
    glBindVertexArray(0);
    
    // VBO off
    glDisableVertexAttribArray(POS_ATTRIB_LOCATION);
    glDisableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
    glDisableVertexAttribArray(COLOR_ATTRIB_LOCATION);
    glDisableVertexAttribArray(TEX_COORD_ATTRIB_LOCATION);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    
    // shader off
    glUseProgram (0);
    
    CHECK_GL_ERRORS();
}
