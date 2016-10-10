#include "UIManager.h"
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shaders.h"
#include "Helpers.h"


UIManager(int width, int height){
    resize(width, height);

    // Шейдеры
    _shaderProgram = createUIShader();
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
    // TODO: Включение шейдера
    for(const UIElementPtr& element: _items){
        element->draw(_projectionMatrix, );
    }
}
