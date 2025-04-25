/*#version 400 core

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
}*/

#version 330 core

in vec2 TexCoords;
in vec3 FragPosTangent;
in vec3 LightDirTangent;
in vec3 ViewDirTangent;

out vec4 FragColor;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;

uniform vec3 lightColor;

void main()
{
    // Normal mapping: sample and transform
    vec3 normal = texture(texture_normal, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0); // map from [0,1] to [-1,1]

    // Lighting vectors (already in tangent space)
    vec3 lightDir = normalize(LightDirTangent);
    vec3 viewDir = normalize(ViewDirTangent - FragPosTangent);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(texture_diffuse, TexCoords).rgb;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * texture(texture_specular, TexCoords).rgb;

    // Final color
    vec3 result = (diffuse + specular) * lightColor;
    FragColor = vec4(result, 1.0);
}