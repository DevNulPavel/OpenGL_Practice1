#ifndef _3DMANAGER_
#define _3DMANAGER_

#include <memory>
#include <list>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <glm.hpp>
#include "Helpers.h"

using namespace std;
using namespace glm;


class RenderManager{
public:
    RenderManager(int width, int height);
    ~RenderManager();
    void resize(int width, int height);
    void draw(float delta);

    SETTER_GETTER(bool, _enableAutoRotate, EnableAutoRotate);
    SETTER_GETTER(vec3, _modelPos, ModelPos);
    SETTER_GETTER(float, _xAngle, XAngle);
    SETTER_GETTER(float, _yAngle, YAngle);
    SETTER_GETTER(float, _zAngle, ZAngle);
    SETTER_GETTER(float, _size, Size);

private:
    GLuint _shaderProgram;

    int _modelViewProjMatrixLocation;
    int _modelViewMatrixLocation;
    int _normalMatrixLocation;
    int _lightPosViewSpaceLocation;
    int _texture1Location;

    GLuint _vbo;
    GLuint _vao;
    size_t _modelVertexCount;
    
    uint _textureId;
    
    vec3 _lightPosWorldSpace;
    mat4 _viewMatrix;
    mat4 _projectionMatrix;
    
    bool _enableAutoRotate;
    vec3 _modelPos;
    float _xAngle;
    float _yAngle;
    float _zAngle;
    float _size;
};

#endif
