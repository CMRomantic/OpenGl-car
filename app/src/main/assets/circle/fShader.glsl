#version 300 es

precision mediump float;
out vec4 fragColor;

uniform vec4 outColor;

void main()
{
    //fragColor = vec4 (0.6, 0.4, 0.5, 1.0);
    fragColor = outColor;
}