//
// Created by cmeng on 2023/4/12.
//

#include "../include/GLUtils.h"
#include <malloc.h>

#define CAR_LOG_TAG "GLUtils"

GLuint GLUtils::loadShader(GLenum shaderType, const char *pSource) {
    LOGD("load shader start");
    GLuint shader = glCreateShader(shaderType);
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
    LOGD("load shader end");
    return shader;
}

GLuint GLUtils::createProgram(
        const char *pVertexShaderSource,
        const char *pFragShaderSource,
        GLuint &vertexShaderHandle,
        GLuint &fragShaderHandle
) {
    LOGD("create program start");
    GLuint program = 0;
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
    LOGD("create program end program = %d", program);
    return program;
}
