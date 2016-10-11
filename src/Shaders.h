#ifndef SHADERS_H
#define SHADERS_H

#include <utility>
#include <map>
#include <string>
#include <GL/glew.h>        // для поддержки расширений, шейдеров и так далее

using namespace std;

GLuint create3DShader(const map<string,int>& attributeLocations);
GLuint createUIShader(const map<string,int>& attributeLocations);

#endif
