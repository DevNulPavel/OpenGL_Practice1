#ifndef UIMANAGER
#define UIMANAGER

#include <memory>
#include <list>
#include <glm.hpp>
#include "UIElement.h"

using namespace std;
using namespace glm;

#define UI_POS_ATTRIBUTE_LOCATION 0
#define UI_TEX_COORD_ATTRIBUTE_LOCATION 1

class UIManager{
public:
    UIManager(int width, int height);
    ~UIManager();
    void resize(int width, int height);
    void addElement(const UIElementPtr& element);
    void removeElement(const UIElementPtr& element);
    void draw(float delta);

private:
    int _shaderProgram;
    uint _texture0Location;
    uint _matrixLocation;
    
    mat4 _projectionMatrix;
    list<UIElementPtr> _items;
};

#endif
