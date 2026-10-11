#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord; // Switched from aColor to texture coordinates

out vec2 TexCoords;

void main()
{
    // The quad is already in Normalized Device Coordinates (-1 to 1)
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;
}