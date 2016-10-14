#include "UIElement.h"
#include <vector>
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
    _texture(0),
    _callback(nullptr){
        
    _anchor = vec2(0.5f);
    _scale = vec2(1.0f);
        
    // create
    loadTexture(imagePath);
    createVAO();
}

UIElement::~UIElement(){
    glDeleteTextures(1, &_texture);
    glDeleteBuffers(1, &_vbo);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,              // формат внутри OpenGL
                     info.width, info.height, 0,            // ширинна, высота, границы
                     info.withAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, info.data); // формат входных данных
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERRORS();
    }
}

void UIElement::createVAO(){
    vector<UIElementVertex> vertexes;
    vertexes.reserve(4);
    
    // вбиваем данные о вершинах
    vertexes.push_back(UIElementVertex(vec2(0, _size.y),        vec2(0, 1)));
    vertexes.push_back(UIElementVertex(vec2(0, 0),              vec2(0, 0)));
    vertexes.push_back(UIElementVertex(vec2(_size.x, _size.y),  vec2(1, 1)));
    vertexes.push_back(UIElementVertex(vec2(_size.x, 0),        vec2(1, 0)));
    
    // VBO, данные о вершинах
    _vbo = 0;
    glGenBuffers (1, &_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, _vbo);
    glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(UIElementVertex), (void*)(vertexes.data()), GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
    
}

void UIElement::draw(const mat4& projectionMatrix, uint matrixLocation, uint texture0Location){
    // рассчет матрицы трансоформов
    mat4 modelMatrix(1.0);
    modelMatrix = glm::translate(modelMatrix, vec3(_position.x, _position.y, 0.0));
    modelMatrix = glm::translate(modelMatrix, vec3(-(_anchor.x * _size.x), -(_anchor.y * _size.y), 0.0));
    modelMatrix = glm::scale(modelMatrix, vec3(_scale, 1.0));
    
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
    //      sizeof(Vertex) - размер блока одной информации о вершине
    //      OFFSETOF(Vertex, color) - смещение от начала
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    CHECK_GL_ERRORS();
    
    // VBO enable arrays
    glEnableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glEnableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    CHECK_GL_ERRORS();
    
    // VBO align
    glVertexAttribPointer(UI_POS_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(UIElementVertex), OFFSETOF(UIElementVertex, pos)); // Позиции
    glVertexAttribPointer(UI_TEX_COORD_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(UIElementVertex), OFFSETOF(UIElementVertex, texCoord));    // Текстурные координаты
    CHECK_GL_ERRORS();
    
    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // VBO off
    glDisableVertexAttribArray(UI_POS_ATTRIBUTE_LOCATION);
    glDisableVertexAttribArray(UI_TEX_COORD_ATTRIBUTE_LOCATION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
}

bool UIElement::tapAtPos(const vec2& pos){
    vec2 leftBottom = vec2(_position.x - (_anchor.x*_size.x), _position.y - (_anchor.y*_size.y));
    vec2 rightTop = vec2(_position.x + (_anchor.x*_size.x), _position.y + (_anchor.y*_size.y));
    bool x1 = (pos.x > leftBottom.x);
    bool y1 = (pos.y > leftBottom.y);
    bool x2 = (pos.x < rightTop.x);
    bool y2 = (pos.y < rightTop.y);

    if (x1 && y1 && x2 && y2) {
        if (_callback) {
            _callback();
        }
        return true;
    }
    
    return false;
}
