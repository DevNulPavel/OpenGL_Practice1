#include "PostProcessManager.h"
#include <map>
#include <vector>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include "Helpers.h"
#include "Shaders.h"

// for linux support
#ifndef GL_RGBA8_OES
#define GL_RGBA8_OES GL_RGBA8
#endif


struct PostProcessVertex{
    vec2 pos;
    vec2 texCoord;
    
    // constructor
    PostProcessVertex(const vec2& inPos, const vec2& inTexCoord):
        pos(inPos),
        texCoord(inTexCoord){
    }
};

//////////////////////////////////////////////////////////////////////////////////////////

PostProcessManager::PostProcessManager(int width, int height):
    _size(width, height),
    _oldFBO(0),
    _colorTexture(0),
    _depthTexture(0),
    _fbo(0),
    _vbo(0),
    _shaderProgram(0),
    _texture0Location(0),
    _matrixLocation(0),
    _blurStepLocation(0),
    _curTime(0){
        
    createFBO();
    createDrawObjects();
}

PostProcessManager::~PostProcessManager() {
    glDeleteFramebuffers(1, &_fbo);
    glDeleteTextures(1, &_depthTexture);
    glDeleteTextures(1, &_colorTexture);
    glDeleteBuffers(1, &_vbo);
}

void PostProcessManager::createFBO(){
    // get current
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&_oldFBO);

    // буфферы
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    
    // создание текстуры + подключение к буфферу отрисовки
    glGenTextures(1, &_colorTexture);
    glBindTexture(GL_TEXTURE_2D, _colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,           // формат внутри OpenGL
                 _size.x, _size.y, 0,                 // ширинна, высота, границы
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // формат входных данных
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // подключение
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);
    CHECK_GL_ERRORS();
    
    // создание ренденбуффера глубины + подключение
    /*glGenRenderbuffers(1, &_depth);
     glBindRenderbuffer(GL_RENDERBUFFER, _depth);
     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _size.x, _size.y);
     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth);
     */
    
    // создание текстуры глубины + подключение
    glGenTextures(1, &_depthTexture);
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,           // формат внутри OpenGL
                 _size.x, _size.y, 0,                 // ширинна, высота, границы
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr); // формат входных данных
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // подключение
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
    CHECK_GL_ERRORS();
    
    // restore
    glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);
}

void PostProcessManager::createDrawObjects(){
    // Шейдеры
    map<string, int> attributesLocations;
    attributesLocations["aPos"] = POSTPROC_POS_ATTRIBUTE_LOCATION;
    attributesLocations["aTexCoord"] = POSTPROC_TEX_COORD_ATTRIBUTE_LOCATION;
    _shaderProgram = createPostProcessShader(attributesLocations);
    CHECK_GL_ERRORS();

    // юниформы шейдера
    _matrixLocation = glGetUniformLocation(_shaderProgram, "uModelViewProjMat");
    _texture0Location = glGetUniformLocation(_shaderProgram, "uTexture0");
    _blurStepLocation = glGetUniformLocation(_shaderProgram, "singleStepOffset");
    _brightnessLocation = glGetUniformLocation(_shaderProgram, "brightness");
    
    vector<PostProcessVertex> vertexes;
    vertexes.reserve(4);
    
    // вбиваем данные о вершинах
    vertexes.push_back(PostProcessVertex(vec2(0, _size.y),        vec2(0, 1)));
    vertexes.push_back(PostProcessVertex(vec2(0, 0),              vec2(0, 0)));
    vertexes.push_back(PostProcessVertex(vec2(_size.x, _size.y),  vec2(1, 1)));
    vertexes.push_back(PostProcessVertex(vec2(_size.x, 0),        vec2(1, 0)));
    
    // VBO, данные о вершинах
    _vbo = 0;
    glGenBuffers (1, &_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, _vbo);
    glBufferData (GL_ARRAY_BUFFER, 4 * sizeof(PostProcessVertex), (void*)(vertexes.data()), GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS();
}

void PostProcessManager::grabStart() {
    // сохраняем старый буффер
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&_oldFBO);

    // включаем новый буффер
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    
    // status
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return;
    }
    
    // чистим перед отрисовкой
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessManager::grabEnd() {
    // восстанавливаем старый буффер отрисовки
    glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);
}

void PostProcessManager::draw(float delta){
    _curTime += delta;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Включение шейдера
    glUseProgram(_shaderProgram);
    
    // обязательно во флоатах, чтобы тип матрицы был верным
    mat4 projectionMatrix = glm::ortho(0.0f, _size.x, 0.0f, _size.y);
    
    // выставляем матрицу трансформа в координаты камеры
    glUniformMatrix4fv(_matrixLocation, 1, false, glm::value_ptr(projectionMatrix));
    
    // смещение для блюра
    float period = 1.0f;
    float timeValue = (sin(_curTime/period*M_PI_2) + 1.0f) / 2.0f;
    float blurValue = timeValue * 2.0f;
    glUniform2f(_blurStepLocation, 1.0f/_size.x*blurValue, 1.0f/_size.y*blurValue);
    
    // яркость
    float brightness = 1.0 + timeValue * 2.0f;
    glUniform1f(_brightnessLocation, brightness);
    
    // говорим шейдеру, что текстура будет на 0 позиции (GL_TEXTURE0)
    glUniform1i(_texture0Location, 0);
    
    // активируем нулевую текстуру для для шейдера, включаем эту текстуру
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _colorTexture);
    
    // рисуем
    //      sizeof(Vertex) - размер блока одной информации о вершине
    //      OFFSETOF(Vertex, color) - смещение от начала
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    CHECK_GL_ERRORS();
    
    // VBO enable arrays
    glEnableVertexAttribArray(POSTPROC_POS_ATTRIBUTE_LOCATION);
    glEnableVertexAttribArray(POSTPROC_TEX_COORD_ATTRIBUTE_LOCATION);
    CHECK_GL_ERRORS();
    
    // VBO align
    glVertexAttribPointer(POSTPROC_POS_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PostProcessVertex), OFFSETOF(PostProcessVertex, pos)); // Позиции
    glVertexAttribPointer(POSTPROC_TEX_COORD_ATTRIBUTE_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PostProcessVertex), OFFSETOF(PostProcessVertex, texCoord));    // Текстурные координаты
    CHECK_GL_ERRORS();
    
    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // VBO off
    glDisableVertexAttribArray(POSTPROC_POS_ATTRIBUTE_LOCATION);
    glDisableVertexAttribArray(POSTPROC_TEX_COORD_ATTRIBUTE_LOCATION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDisable(GL_BLEND);
    glUseProgram (0);
}

