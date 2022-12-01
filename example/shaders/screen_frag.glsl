#version 330 core

layout (location = 0) out vec4 OutColor;

in vec3 Color;

uniform vec3 uColor;

void main()
{
	OutColor = vec4(Color * uColor, 1.0);
}