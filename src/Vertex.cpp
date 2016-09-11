#include "Vertex.h"

Vertex::Vertex():
    pos(vec3(0.0, 0.0, 0.0)),
    normal(vec3(0.0, 0.0, 0.0)),
    color(vec3(1.0, 1.0, 1.0)),
    texCoord(vec2(0.0, 0.0)){

}

Vertex::Vertex(vec3 inPos, vec3 inNormal, vec2 inTexCoord):
    pos(inPos),
    normal(inNormal),
    color(vec3(1.0, 1.0, 1.0)),
    texCoord(inTexCoord){
}

Vertex::Vertex(vec3 inPos, vec3 inNormal, vec3 inColor, vec2 inTexCoord):
    pos(inPos),
    normal(inNormal),
    color(inColor),
    texCoord(inTexCoord){
}

