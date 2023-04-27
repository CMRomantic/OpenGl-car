//
// Created by cmeng on 2023/4/25.
//

#include <GLES3/gl3.h>
#include <jni.h>

#ifndef OPENGL_GLRENDER_H
#define OPENGL_GLRENDER_H


class GLRender {
    GLRender() {
        programObj = GL_NONE;
        VAO = GL_NONE;
    };

    ~GLRender() {
        if (context != nullptr) {
            delete context;
            context = nullptr;
        }
        if (vShaderStr != nullptr) {
            delete vShaderStr;
            vShaderStr = nullptr;
        }
        if (fShaderStr != nullptr) {
            delete fShaderStr;
            fShaderStr = nullptr;
        }
    };
public:
    void init(const char *vShader, const char *fShader) {
        this->vShaderStr = vShader;
        this->fShaderStr = fShader;
    }

    static void OnSurfaceCreated();

    static void OnSurfaceChanged(int width, int height);

    void OnDrawFrame();

    void beforeDraw();

    void destroy();

    static GLRender *getInstance();

private:
    static GLRender *context;
    const char *vShaderStr;
    const char *fShaderStr;
    GLuint programObj;
    GLuint VAO;
};


#endif //OPENGL_GLRENDER_H
