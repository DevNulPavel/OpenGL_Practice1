#include "UIElement.h"
#include <vector>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <glm.hpp>          // библиотека графической математики
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "PngLoader.h"
#include "Helpers.h"
#include "UIManager.h"

using namespace glm;

struct UIElementVertex{
    vec2 pos;
    vec2 texCoord;
    
    // constructor
    UIElementVertex(const vec2& inPos, const vec2& inTexCoord):
        pos(inPos),
        texCoord(inTexCoord){
    }
};

///////////////////////////////////////////////////////////////////////////

UIElement::UIElement(const string& imagePath):
    _vbo(0),
    _vao(0),
    _texture(0),
    _callback(nullptr){
        
    // create
    loadTexture(imagePath);
    createVAO();
}

UIElement::~UIElement(){
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}

void UIElement::loadTexture(const string& path){
    // Загрузка текстуры
    ImageData info = loadPngImage(path.c_str());
    
    _texture = 0;
    if(info.loaded){
        // обновление данных размера
        _size = vec2(info.width, info.height);
        
        // создание текстуры
        glGenTextures(1, &_texture);
        glBindTexture(GL_TEXTURE_2D, _texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,              // формат внутри OpenGL
                     info.width, info.height, 0,            // ширинна, высота, границы
                     info.withAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, info.data); // формат входных данных
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECK_GL_ERRORS();
    }
}

void UIElement::createVAO(){
    vector<UIElementVertex> vertexes;
    vertexes.reserve(4);
    
    // вбиваем данные о вершинах
    vertexes.push_back(UIElementVertex(vec2(0, 0),              vec2(0, 0)));
    vertexes.push_back(UIElementVertex(vec2(0, _size.y),        vec2(0, 1)));
    vertexes.push_back(UIElementVertex(vec2(_size.x, _size.y),  vec2(1, 1)));
    vertexes.push_back(UIElementVertex(vec2(_size.x, 0),        vec2(1, 0)));
    
    // VBO, данные о вершинах
    _vbo = 0;
    glGenBuffers (1, &_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, _vbo);
    glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(UIElementVertex), (void*)(vertexes.data()), GL_STATIC_DRAW);
    CHECK_GL_ERRORS();
    
    // VAO
    _vao = 0;
    glGenVertexArrays (1, &_vao);
    glBindVertexArray (_vao);
    // sizeof(Vertex) - размер блока одной информации о вершине
    // OFFSETOF(Vertex, color) - смещение от начала
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    // Позиции
    glEnableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glVertexAttribPointer(UI_POS_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(UIElementVertex), OFFSETOF(UIElementVertex, pos));
    // Текстурные координаты
    glEnableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    glVertexAttribPointer(UI_TEX_COORD_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(UIElementVertex), OFFSETOF(UIElementVertex, texCoord));
    // off
    glBindVertexArray(0);
    CHECK_GL_ERRORS();
}

void UIElement::draw(const mat4& projectionMatrix, uint matrixLocation, uint texture0Location){
    // рассчет матрицы трансоформов
    mat4 modelMatrix(1.0);
    modelMatrix = glm::translate(modelMatrix, vec3(_position.x, _position.y, 0.0));
    modelMatrix = glm::translate(modelMatrix, vec3(-(_anchor.x * _size.x), -(_anchor.y * _size.y), 0.0));
    
    // финальная матрица
    mat4 resultMatrix = projectionMatrix * modelMatrix;

    // выставляем матрицу трансформа в координаты камеры
    glUniformMatrix4fv(matrixLocation, 1, false, glm::value_ptr(resultMatrix));
    
    // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
    glUniform1i(texture0Location, 0);
    
    // активируем нулевую текстуру для для шейдера, включаем эту текстуру
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    
    // рисуем
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
