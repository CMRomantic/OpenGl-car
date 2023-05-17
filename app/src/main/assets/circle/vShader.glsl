#version 300 es
//顶点着色器
layout(location = 0) in vec4 vPosition;
void main()
{
    gl_Position = vPosition;
}
