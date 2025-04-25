#version 400 core

in vec2 uv;
in vec3 normal;

uniform sampler2D tex;

void main(void)
{
	vec4 ambientLight = vec4(.1, .1, .1, 1);
	vec4 lightColor = vec4(1, .9, .5, 1);
	vec3 lightDir = vec3(-1, -1, -2);

	// calculate diffuse lighting and clamp between 0 and 1
	float ndotl = clamp(-dot(normalize(lightDir), normalize(normal)), 0, 1); 

	// add diffuse lighting to ambient lighting and clamp a second time
	vec4 lightValue = clamp(lightColor * ndotl + ambientLight, 0, 1);

	// finally, sample from the texuture and multiply in the light.
	gl_FragColor = texture(tex, uv) * lightValue;
}

/*#version 400 core

in vec2 uv;
in vec3 normal;
in vec3 fragPos; // Fragment position in world space

// Input textures
uniform sampler2D texDiffuse;  // Diffuse texture
uniform sampler2D texSpecular; // Specular map
uniform sampler2D texNormal;   // Normal map

// Camera and lighting
uniform vec3 viewPos;        // Camera position for specular calculation

void main(void)
{
    // Light properties
    vec4 ambientLight = vec4(0.1, 0.1, 0.1, 1.0);
    vec4 lightColor = vec4(1.0, 0.9, 0.5, 1.0);
    vec3 lightDir = normalize(vec3(-1.0, -1.0, -2.0));
    
    // Get texture values
    vec4 diffuseColor = texture(texDiffuse, uv);
    vec4 specularMap = texture(texSpecular, uv);
    
    // Process normal map
    vec3 normalMap = texture(texNormal, uv).rgb;
    // Transform normal map from [0,1] to [-1,1] range
    normalMap = normalMap * 2.0 - 1.0;
    
    // Use the normal from the normal map, or fall back to the vertex normal if no normal map is provided
    // In practice, you'd use a TBN matrix to transform the normal map to world space
    vec3 N = length(normalMap) > 0.5 ? normalize(normalMap) : normalize(normal);
    
    // Calculate diffuse lighting
    float ndotl = max(-dot(N, lightDir), 0.0);
    vec4 diffuseLight = lightColor * ndotl;
    
    // Calculate specular lighting
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(lightDir, N);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec4 specularLight = lightColor * spec * specularMap;
    
    // Combine all lighting components
    vec4 lightValue = clamp(diffuseLight + ambientLight, 0.0, 1.0);
    vec4 specValue = clamp(specularLight, 0.0, 1.0);
    
    // Final color combines diffuse texture with diffuse lighting and adds specular
    gl_FragColor = (diffuseColor * lightValue) + specValue;
}*/