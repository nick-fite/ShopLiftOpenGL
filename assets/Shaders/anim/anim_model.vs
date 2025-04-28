#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;

out vec4 debugColor;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    float weightSum = 0.0f;
    
    // Process bone transformations
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
            continue;
            
        weightSum += weights[i];
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0f);
        totalPosition += localPosition * weights[i];
        //debugColor = vec4(0.0, totalPosition.z, 0.0, 1.0);  // Red if any bone influence
    }
    
    // Use original position if no weights were applied
    if(weightSum < 0.001f)
        totalPosition = vec4(pos, 1.0f);
        
    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;
    TexCoords = tex;

    if(weights.x != -1 || weights.y != -1 || weights.z != -1 || weights.w != -1) {
        //debugColor = vec4(1.0, 0.0, 0.0, 1.0);  // Red if any bone influence
    }
    else {
        debugColor = vec4(0.0, 0.0, 1.0, 1.0);  // Blue if no bone influences
    }
}