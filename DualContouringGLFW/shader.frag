#version 400

smooth in vec3 vertexColour;
smooth in vec3 vertexNormal;
out vec4 FragColor;

uniform vec4 colour;

void main()
{
	vec3 lightDir = -normalize(vec3(1, 5, -5));
	float d = dot(vertexNormal, -lightDir);
	d = max(0.2, d);
	FragColor = vec4(vertexColour, 1) * d;
}