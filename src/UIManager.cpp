#include "UIManager.h"
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shaders.h"
#include "Helpers.h"


UIManager::UIManager(float width, float height){
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
    _texture0Location = glGetUniformLocation(_shaderProgram, "uTexture0");
    CHECK_GL_ERRORS();
}

UIManager::~UIManager(){
    // удаление
    glDeleteProgram(_shaderProgram);
}

void UIManager::resize(float width, float height){
    // обязательно во флоатах, чтобы тип матрицы был верным
    _projectionMatrix = glm::ortho(0.0f, width, 0.0f, height);
}

void UIManager::addElement(const UIElementPtr& element){
    _items.push_back(element);
}

void UIManager::removeElement(const UIElementPtr& element){
    _items.remove(element);
}

void UIManager::draw(float delta){
    glDisable(GL_DEPTH_TEST);

    // Включение шейдера
    glUseProgram (_shaderProgram);

    for(const UIElementPtr& element: _items){
        element->draw(_projectionMatrix, _matrixLocation, _texture0Location);
    }
    
    glUseProgram (0);
}
