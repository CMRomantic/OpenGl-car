#version 300 es

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;
out vec3 color;

void main() {
    gl_Position = vPosition;
    color = vColor;
}