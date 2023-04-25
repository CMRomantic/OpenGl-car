//
// Created by cmeng on 2023/4/25.
//

#include <GLES3/gl3.h>
#include <jni.h>

#ifndef OPENGL_GLRENDER_H
#define OPENGL_GLRENDER_H


class GLRender {
    GLRender() {

    };

    ~GLRender() {
        if (context != nullptr) {
            delete context;
            context = nullptr;
        }
    };
public:
    static void OnSurfaceCreated();

    static void OnSurfaceChanged(int width, int height);

    void OnDrawFrame();

    void beforeDraw();

    void destroy();

    static GLRender *getInstance();

private:
    static GLRender *context;
    int mScreenWidth;
    int mScreenHeight;
    GLuint programObj;
    GLuint VAO;
};


#endif //OPENGL_GLRENDER_H
