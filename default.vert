#version 430 core
layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float pointSize;

void main()
{
	gl_PointSize = pointSize;
	gl_Position = projection * view * model * vec4(aPos.xyz, 1.0);
	ourColor = aColor.rgb;
}