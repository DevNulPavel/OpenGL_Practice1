#ifndef UI_ELEMENT
#define UI_ELEMENT

#include <memory>
#include <string>
#include <functional>
#include <glm.hpp>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include "Helpers.h"

using namespace std;
using namespace glm;


class UIElement{
public:
    UIElement(const string& imagePath);
    ~UIElement();

    SETTER_GETTER(vec2, _position, Position);
    SETTER_GETTER(vec2, _anchor, Anchor);
    SETTER_GETTER(vec2, _scale, Scale);
    SETTER_GETTER(function<void()>, _callback, Callback);

    void draw(const mat4& projectionMatrix, uint matrixLocation, uint texture0Location);
    bool tapAtPos(const vec2& pos);
    

public:
    GLuint _vbo;
    GLuint _texture;
    
    vec2 _size;
    vec2 _position;
    vec2 _anchor;
    vec2 _scale;
    float _angle;
    
    function<void()> _callback;

private:
    void loadTexture(const string& path);
    void createVAO();
};

typedef shared_ptr<UIElement> UIElementPtr;

#endif
