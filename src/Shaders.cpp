#include "Shaders.h"
#include "Helpers.h"


GLuint createShaderFromSources(const char* vertexShader, const char* fragmentShader){
    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    CHECK_GL_ERRORS();

    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    CHECK_GL_ERRORS();

    GLuint shaderProgram = glCreateProgram ();
    glAttachShader(shaderProgram, fs);
    glAttachShader(shaderProgram, vs);
    glLinkProgram(shaderProgram);
    CHECK_GL_ERRORS();

    return shaderProgram;
}

GLuint createShader(){
    // Шейдер вершин
    const char* vertexShader = STRINGIFY_SHADER(
        // vertex attribute
        attribute vec3 aPos;
        attribute vec3 aColor;
        // uniforms
        uniform mat4 uModelViewProjMat;
        // output
        varying vec3 vColor;

        void main () {
            vec4 vertexVec4 = vec4(aPos, 1.0);      // последняя компонента 1, тк это точка
            // вычисляем позицию точки в пространстве OpenGL
            gl_Position = uModelViewProjMat * vertexVec4;
            // цвет и текстурные координаты просто пробрасываем для интерполяции
            vColor = aColor;
        }
    );
    const char* fragmentShader = STRINGIFY_SHADER(
        varying vec3 vColor;

        void main () {
            gl_FragColor = vec4(vColor, 1.0);
        }
    );

    GLuint shader = createShaderFromSources(vertexShader, fragmentShader);
    CHECK_GL_ERRORS();
    return shader;
}

