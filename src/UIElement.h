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
    UIElement(const string& imagePath, const function<void()>& callback = function<void()>());
    ~UIElement();

    void draw(const mat4& projectionMatrix);

public:
    int _vao;
    int _texture;
    vec2 _size;
    vec2 _position;
    vec2 _anchor;
    function<void()> _callback;
};

typedef shared_ptr<UIElement> UIElementPtr;

#endif
