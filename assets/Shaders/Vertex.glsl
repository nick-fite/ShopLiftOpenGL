#version 400 core

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
}

/*#version 400 core

// Vertex attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tangent;  // Tangent vector for normal mapping

// Uniforms for transformations
uniform mat4 worldMatrix;
uniform mat4 cameraView;

// Outputs to fragment shader
out vec2 uv;
out vec3 normal;
out vec3 fragPos;      // Fragment position in world space
out mat3 TBN;          // Tangent-Bitangent-Normal matrix for normal mapping

void main(void)
{
    // Transform the vertex position
    vec4 worldPosition = worldMatrix * vec4(in_position, 1.0);
    fragPos = worldPosition.xyz;  // Pass world position to fragment shader
    
    // Final position
    gl_Position = cameraView * worldPosition;
    
    // Transform normal to world space
    normal = normalize(mat3(worldMatrix) * in_normal);
    
    // Calculate TBN matrix for normal mapping
    vec3 T = normalize(mat3(worldMatrix) * in_tangent);
    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, normal) * normal);
    vec3 B = cross(normal, T);  // Create bitangent
    
    // Output TBN matrix
    TBN = mat3(T, B, normal);
    
    // Pass texture coordinates
    uv = in_uv;
}*/