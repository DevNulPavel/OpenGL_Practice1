#ifndef UIMANAGER
#define UIMANAGER

#include <memory>
#include <list>
#include <glm.hpp>
#include "UIElement.h"

using namespace std;
using namespace glm;

class UIManager{
public:
    UIManager(int width, int height);
    ~UIManager();
    void resize(int width, int height);
    void addElement(const UIElementPtr& element);
    void removeElement(const UIElementPtr& element);
    void draw();

private:
    mat4 _projectionMatrix;
    int _shaderProgram;
    list<UIElementPtr> _items;
};

#endif
