//
// Created by cmeng on 2023/4/12.
//

#include <GLES3/gl3.h>
#include <string>
#include "../config.h"

#ifndef OPENGL_GLUTILS_H
#define OPENGL_GLUTILS_H


class GLUtils {
public:
    static GLuint loadShader(GLenum shaderType, const char *pSource);

    static GLuint createProgram(
            const char *pVertexShaderSource,
            const char *pFragShaderSource,
            GLuint &vertexShaderHandle,
            GLuint &fragShaderHandle);
};


#endif //OPENGL_GLUTILS_H
