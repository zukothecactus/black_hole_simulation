#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

// The output texture from the compute shader
uniform sampler2D screenTexture; 

void main()
{
    // Sample the exact pixel color calculated by the compute shader
    FragColor = texture(screenTexture, TexCoords);
}