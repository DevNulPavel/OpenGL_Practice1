#ifndef FIGURES_H
#define FIGURES_H

#include <glm.hpp>
#include "Vertex.h"

using namespace glm;

// данные о вершинах
int piramideVertexCount = 18;
Vertex piramideVertexes[] = {
    //              ВЕРШИНА                     НОРМАЛЬ                 ЦВЕТ                ТЕКСТУРНЫЕ_КООРДИНАТЫ
    // первая грань, передняя
    Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
    Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.0f,  0.5f,  0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
    // вторая, правая
    Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
    Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.5f,  0.5f,  0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
    // третья, задняя
    Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
    Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3( 0.0f,  0.5f, -0.5f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
    // четвертая, левая
    Vertex(vec3( 0.0f,  1.0f,  0.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.5f, 1.0f)),
    Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
    // пятая, низ пирамиды 1
    Vertex(vec3(-1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)),
    Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 1.0f)),
    // шестая, низ пирамиды 2
    Vertex(vec3( 1.0f, -1.0f, -1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 0.0f)),
    Vertex(vec3( 1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 1.0f)),
    Vertex(vec3(-1.0f, -1.0f,  1.0f), vec3( 0.0f, -1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 1.0f))
};

#endif
