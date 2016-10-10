#include "UIElement.h"


UIElement::UIElement(const string& imagePath, const function<void()>& callback):
    _vao(0),
    _texture(0),
    _callback(callback){

}

UIElement::~UIElement(){

}

void UIElement::draw(const mat4& projectionMatrix){

}
