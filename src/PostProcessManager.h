#ifndef POST_PROCESS_NODE
#define POST_PROCESS_NODE

#define _USE_MATH_DEFINES // for math.h

#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее
#include <glm.hpp>

using namespace glm;
using namespace std;

#define POSTPROC_POS_ATTRIBUTE_LOCATION 0
#define POSTPROC_TEX_COORD_ATTRIBUTE_LOCATION 1

class PostProcessManager{
public:
	PostProcessManager(int width, int height);
    ~PostProcessManager();

	void grabStart();
	void grabEnd();
    void draw(float delta);
	
private:
    vec2 _size;
    
    uint _oldFBO;
    
    uint _colorTexture;
    uint _depthTexture;
	uint _fbo;
    
    uint _vbo;
    int _shaderProgram;
    int _texture0Location;
    int _matrixLocation;
    int _blurStepLocation;
    int _brightnessLocation;
    
    float _curTime;

private:
    void createFBO();
    void createDrawObjects();
};


#endif
