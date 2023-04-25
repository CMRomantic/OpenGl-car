//
// Created by cmeng on 2023/4/12.
//

#include "GLUtils.h"
#include <malloc.h>

#define CAR_LOG_TAG "GLUtils"

void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

void GLUtils::initGl() {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    //glShadeModel(GL_SMOOTH);// 启用阴影平滑
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// 黑色背景
    glClearDepthf(1.0f);// 设置深度缓存
    glEnable(GL_DEPTH_TEST);// 启用深度测试
    glDepthFunc(GL_LEQUAL);// 所作深度测试的类型
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// 告诉系统对透视进行修正
}

GLuint GLUtils::loadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = 0;
    FUN_BEGIN_TIME("GLUtils::loadShader")
        shader = glCreateShader(shaderType);
        if (shader) {
            glShaderSource(shader, 1, &pSource, nullptr);
            glCompileShader(shader);
            GLint params = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
            if (!params) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char *buff = (char *) malloc((size_t) infoLen);
                    if (buff) {
                        glGetShaderInfoLog(shader, infoLen, nullptr, buff);
                        LOGE("load shader could not compile shader %d:\n%s\n", shaderType, buff);
                        free(buff);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
    FUN_END_TIME("GLUtils::loadShader")
    return shader;
}

GLuint GLUtils::createProgram(
        const char *pVertexShaderSource,
        const char *pFragShaderSource,
        GLuint &vertexShaderHandle,
        GLuint &fragShaderHandle
) {
    GLuint program = 0;
    FUN_BEGIN_TIME("GLUtils::createProgram")
        vertexShaderHandle = loadShader(GL_VERTEX_SHADER, pVertexShaderSource);
        if (!vertexShaderHandle) {
            return program;
        }
        fragShaderHandle = loadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
        if (!fragShaderHandle) {
            return program;
        }

        program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShaderHandle);

            glAttachShader(program, fragShaderHandle);

            glLinkProgram(program);

            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

            glDetachShader(program, vertexShaderHandle);
            glDeleteShader(vertexShaderHandle);
            vertexShaderHandle = 0;
            glDetachShader(program, fragShaderHandle);
            glDeleteShader(fragShaderHandle);
            fragShaderHandle = 0;
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc((size_t) bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, nullptr, buf);
                        LOGE("create program Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
    FUN_END_TIME("GLUtils::createProgram")
    LOGD("create program end program = %d", program);
    return program;
}

GLuint GLUtils::createProgram(const char *pVertexShaderSource, const char *pFragShaderSource) {
    GLuint vertexShaderHandle, fragShaderHandle;
    return createProgram(pVertexShaderSource, pFragShaderSource, vertexShaderHandle,
                         fragShaderHandle);
}

GLuint
GLUtils::createProgramWithFeedback(
        const char *pVertexShaderSource,
        const char *pFragShaderSource,
        GLuint &vertexShaderHandle,
        GLuint &fragShaderHandle,
        const GLchar **varying, int varyingCount) {

    GLuint program = 0;
    FUN_BEGIN_TIME("GLUtils::createProgramWithFeedback")
        vertexShaderHandle = loadShader(GL_VERTEX_SHADER, pVertexShaderSource);
        if (!vertexShaderHandle) return program;

        fragShaderHandle = loadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
        if (!fragShaderHandle) return program;

        program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShaderHandle);
            checkGLError("glAttachShader");
            glAttachShader(program, fragShaderHandle);
            checkGLError("glAttachShader");

            //transform feedback
            glTransformFeedbackVaryings(program, varyingCount, varying, GL_INTERLEAVED_ATTRIBS);
            GO_CHECK_GL_ERROR();

            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

            glDetachShader(program, vertexShaderHandle);
            glDeleteShader(vertexShaderHandle);
            vertexShaderHandle = 0;
            glDetachShader(program, fragShaderHandle);
            glDeleteShader(fragShaderHandle);
            fragShaderHandle = 0;
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc((size_t) bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, nullptr, buf);
                        LOGE("GLUtils::CreateProgramWithFeedback Could not link program:\n%s\n",
                             buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
    FUN_END_TIME("GLUtils::createProgramWithFeedback")
    LOGD("GLUtils::CreateProgramWithFeedback program = %d", program);
    return program;
}

void GLUtils::deleteProgram(GLuint &program) {
    LOGD("GLUtils::deleteProgram");
    if (program) {
        glUseProgram(0);
        glDeleteProgram(program);
        program = 0;
    }
}

void GLUtils::checkGLError(const char *pGLOperation) {
    for (GLenum error = glGetError(); error; error = glGetError()) {
        LOGE("GLUtils::checkGLError GL Operation %s() glError (0x%x)\n", pGLOperation, error);
    }
}