#ifndef UI_ELEMENT
#define UI_ELEMENT

#include <memory>
#include <string>
#include <functional>
#include <glm.hpp>

using namespace std;
using namespace glm;


class UIElement{
public:
    UIElement(const string& imagePath);
    ~UIElement();

    void draw(const mat4& projectionMatrix, uint matrixLocation, uint texture0Location);

public:
    uint _vbo;
    uint _vao;
    uint _texture;
    
    vec2 _size;
    vec2 _position;
    vec2 _anchor;
    function<void()> _callback;

private:
    void loadTexture(const string& path);
    void createVAO();
};

typedef shared_ptr<UIElement> UIElementPtr;

#endif
