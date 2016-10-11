#include "UIManager.h"
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shaders.h"
#include "Helpers.h"


UIManager::UIManager(int width, int height){
    // изменение размера
    resize(width, height);

    // Шейдеры
    map<string, int> attributesLocations;
    attributesLocations["aPos"] = UI_POS_ATTRIBUTE_LOCATION;
    attributesLocations["aTexCoord"] = UI_TEX_COORD_ATTRIBUTE_LOCATION;
    _shaderProgram = createUIShader(attributesLocations);
    CHECK_GL_ERRORS();
    
    // юниформы шейдера
    _matrixLocation = glGetUniformLocation(_shaderProgram, "uModelViewProjMat");
    _texture0Location = glGetUniformLocation(_shaderProgram, "uTexture1");
    CHECK_GL_ERRORS();
}

UIManager::~UIManager(){
    // удаление
    glDeleteProgram(_shaderProgram);
}

void UIManager::resize(int width, int height){
    _projectionMatrix = glm::ortho(0, width, 0, height);
}

void UIManager::addElement(const UIElementPtr& element){
    _items.push_back(element);
}

void UIManager::removeElement(const UIElementPtr& element){
    _items.remove(element);
}

void UIManager::draw(){
    // Включение шейдера
    glUseProgram (_shaderProgram);

    for(const UIElementPtr& element: _items){
        element->draw(_projectionMatrix, _matrixLocation, _texture0Location);
    }
}
