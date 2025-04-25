/*#version 400 core

// Vertex attribute for position
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

// uniform will contain the world matrix.

uniform mat4 worldMatrix;
uniform mat4 cameraView;

out vec2 uv;
out vec3 normal;

void main(void)
{
	//transform the vector
	vec4 worldPosition = worldMatrix * vec4(in_position, 1);
	vec4 viewPosition = cameraView * worldPosition;

	// output the transformed vector
	gl_Position = viewPosition;
	normal = mat3(worldMatrix) * in_normal;
	uv = in_uv;
}*/


#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 FragPosTangent;
out vec3 LightDirTangent;
out vec3 ViewDirTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform vec3 lightDir; // World space

void main()
{
    // Transform position to world space
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));

    // Calculate TBN matrix
    vec3 T = normalize(mat3(model) * aTangent);
    vec3 B = normalize(mat3(model) * aBitangent);
    vec3 N = normalize(mat3(model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N)); // world to tangent

    // Convert to tangent space
    FragPosTangent = TBN * FragPos;
    ViewDirTangent = TBN * viewPos;
    LightDirTangent = TBN * -lightDir;

    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}