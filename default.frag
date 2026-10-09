#version 330 core
out vec4 FragColor;
in vec3 ourColor;

uniform bool isPoint;

void main()
{
	if (isPoint)
    {
        vec2 circCoord = gl_PointCoord - vec2(0.5);
        float dist = length(circCoord);

        if (dist > 0.5) 
        {
            discard;
        }

        float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
        FragColor = vec4(ourColor, alpha);
    }
    else
    {
        FragColor = vec4(ourColor, 1.0);
    }
}