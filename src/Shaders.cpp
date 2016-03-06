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
        attribute vec3 aNormal;
        attribute vec3 aColor;
        attribute vec2 aTexCoord;
        // uniforms
        uniform mat4 uModelViewMat;
        uniform mat4 uNormalMat;
        uniform mat4 uModelViewProjMat;
        // output
        varying vec3 vPosViewSpace;
        varying vec3 vNormalViewSpace;
        varying vec3 vColor;
        varying vec2 vTexCoord;

        void main () {
            vec4 vertexVec4 = vec4(aPos, 1.0);      // последняя компонента 1, тк это точка
            vec4 normalVec4 = vec4(aNormal, 0.0);   // последняя компонента 0, тк это направление
            // вычисляем позицию точки в пространстве OpenGL
            gl_Position = uModelViewProjMat * vertexVec4;
            // вычисляем позицию и нормаль в пространстве камеры
            vPosViewSpace = vec3(uModelViewMat * vertexVec4);                   // это вершина, учитывается перенос и тд
            vNormalViewSpace = normalize(vec3(uNormalMat * normalVec4));    // так как это направление, учитывается поворот только
            // цвет и текстурные координаты просто пробрасываем для интерполяции
            vColor = aColor;
            vTexCoord = aTexCoord;
        }
    );
    const char* fragmentShader = STRINGIFY_SHADER(
        varying vec3 vPosViewSpace;
        varying vec3 vNormalViewSpace;
        varying vec3 vColor;
        varying vec2 vTexCoord;

        uniform sampler2D uTexture1;
        uniform vec3 uLightPosViewSpace;

        const float ambientCoef = 0.5;
        const float diffuseCoef = 0.5;
        const float specularCoeff = 1.0;
        const float specularShinnes = 3.0;  // >0

        void main () {
            // Рассчет освещения
            vec3 fromTexelToLightDir = normalize(uLightPosViewSpace - vPosViewSpace);
            vec3 fromTexelToEyesDir = normalize(-vPosViewSpace);
            vec3 texelLightReflectionDir = normalize(reflect(-fromTexelToLightDir, vNormalViewSpace));
            // Диффузное
            // на сколкьо сильно совпадает направление нормали и направления к свету
            float diffusePower = diffuseCoef * max(dot(vNormalViewSpace, fromTexelToLightDir), 0.0);
            // Блики
            // Степень того, как сильно совпадает направление отражения света и направление в камеру
            float specularDot = max(dot(texelLightReflectionDir, fromTexelToEyesDir), 0.0);
            float specularPower = 0.0;
            // проверка, так как 0 в любой степени - это 1.0
            if(specularDot > 0.0){
               specularPower = pow(specularDot, specularShinnes);
               specularPower = clamp(specularPower, 0.0, 1.0);
            }
            float lightPower = ambientCoef + diffusePower + specularPower;

            // текстура
            vec3 textureColor = vec3(texture2D(uTexture1, vTexCoord));

            gl_FragColor = vec4(textureColor * vColor * lightPower, 1.0);
        }
    );

    GLuint shader = createShaderFromSources(vertexShader, fragmentShader);
    CHECK_GL_ERRORS();
    return shader;
}

