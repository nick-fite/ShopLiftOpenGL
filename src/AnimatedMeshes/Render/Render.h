#include <glm/glm.hpp>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "../ASSIMPFunctions/ASSIMPFunc.h"
#include "../Shader/Shader.h"
#include "../Animation/Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>


struct RenderTexture 
{
    bool loaded = false;
    unsigned int textureName = 0;
    TextureType type = TextureType::None;
    RenderTexture() = default;

    static RenderTexture FromMemory(TextureType type, unsigned width, unsigned height, const void* data, GLenum format)
    {
        if(glfwGetCurrentContext() == nullptr)
        {
            std::fprintf(stderr, "No OpenGL context\n");
            return RenderTexture();
        }
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::fprintf(stderr, "OpenGL error: %d\n", err);
            // Handle error appropriately
        }
        std::fprintf(stderr, "loading textures \n");
        unsigned texture_name = 0;
        glGenTextures(1, &texture_name);
        glBindTexture(GL_TEXTURE_2D, texture_name);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return RenderTexture(texture_name, type);
    }

    static RenderTexture InvalidWhite()
    {
        std::fprintf(stderr, "loading white\n");
        unsigned data = 0xffffff;
        return RenderTexture::FromMemory(TextureType::None, 1, 1, &data, GL_RGB);
    }
    ~RenderTexture() noexcept
    {
        if (std::exchange(loaded, false))
        {
            glDeleteTextures(1, &textureName);
        }
    }
    RenderTexture(RenderTexture&& other) noexcept
        : loaded(std::exchange(other.loaded, false)), textureName(std::exchange(other.textureName, 0)), type(std::exchange(other.type, TextureType::None)){}
    
    RenderTexture& operator=(RenderTexture&& ) = delete;
    RenderTexture(const RenderTexture&) = delete;
private:
    explicit RenderTexture(unsigned int textureName, TextureType type)
        : loaded(true), textureName(textureName), type(type) {}

public:
    static RenderTexture LoadTexture(const AnimTexture& rawTex)
    {
        int width = 0;
        int height = 0;
        int component = 0;
        unsigned char* data = nullptr;
        if(rawTex.memoryTexture.png.size() > 0)
        {
            data = stbi_load_from_memory(rawTex.memoryTexture.png.data(), 
                int(rawTex.memoryTexture.png.size()), 
                &width, &height, &component, 0);
        }
        else
        {
            data = stbi_load(rawTex.path.string().c_str(),
                &width, &height, &component, 0);
        }
        assert(data);
        GLenum format = GL_RED;
        switch(component)
        {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default: assert(false); break;
        }
        std::fprintf(stderr, "loading textures \n");
        RenderTexture texture = RenderTexture::FromMemory(rawTex.type, width, height, data, format);
        stbi_image_free(data);
        return texture;
    }

};

using TextureHandle = int;

struct TexturesDB
{
    RenderTexture invalid;
    std::vector<RenderTexture> textures;
    
    TextureHandle add(RenderTexture&& texture)
    {
        textures.push_back(std::move(texture));
        return TextureHandle(textures.size() - 1);
    }

    unsigned get(TextureHandle handle) const
    {
        if (handle < 0 || handle >= textures.size())
        {
            return invalid.textureName;
        }
        return textures[std::size_t(handle)].textureName;
    }
};

class RenderMesh
{
public:
    TextureHandle diffuse;
    TextureHandle normal;

    static RenderMesh FromMemory(std::vector<AnimVertex>&& vertices, std::vector<unsigned>&& indices, TextureHandle diffuse, TextureHandle normal)
    {
        assert(vertices.size() > 0);
        assert(indices.size() > 0);

        unsigned VAO = 0;
        unsigned VBO = 0;
        unsigned EBO = 0;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(AnimVertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, texCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, m_BoneIDs));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, m_Weights));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, tangent));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(AnimVertex), (void*)offsetof(AnimVertex, bitangent));
        glBindVertexArray(0);

        return RenderMesh(VAO, VBO, EBO, indices.size(), diffuse, normal);
    }

    RenderMesh(const RenderMesh&) = delete;
    RenderMesh& operator=(const RenderMesh&) = delete;
    RenderMesh& operator=(RenderMesh&&) = delete;
    RenderMesh(RenderMesh&& other) noexcept
        : VAO(std::exchange(other.VAO, 0)), VBO(std::exchange(other.VBO, 0)), EBO(std::exchange(other.EBO, 0)), 
        indicesCount(std::exchange(other.indicesCount, 0)), diffuse(std::move(other.diffuse)), normal(std::move(other.normal)) {}
    ~RenderMesh();
    void draw(TexturesDB& textures, int diffusePtr, int normalPtr, int debugFlagPtr);


private:
    unsigned VAO;
    unsigned VBO;
    unsigned EBO;
    std::size_t indicesCount;

    explicit RenderMesh(unsigned VAO, unsigned VBO, unsigned EBO, std::size_t indicesCount, TextureHandle diffuse, TextureHandle normal)
        : VAO(VAO), VBO(VBO), EBO(EBO), indicesCount(indicesCount), diffuse(diffuse), normal(normal) {}
};


struct RenderModel
{
    static RenderModel MakeSimpleNormalMapping(TexturesDB&& textures, std::vector<RenderMesh>&& meshes)
    {
        //TODO: REPLACE WITH FILE SYSTEM
        const char* VertexShader = R"(
#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in ivec4 in_BoneIds;
layout(location = 4) in vec4 in_Weights;
layout(location = 5) in vec3 in_Tangent;
layout(location = 6) in vec3 in_Bitangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// See kMaxBonesCount.
uniform mat4 bone_transforms[100];

out vec2 v_UV;
out vec3 v_Position;
out vec3 v_Normal;
out mat3 v_TBN;
// Debug: highlight with green color.
flat out int v_Highlight;

void main()
{
    int debug_bone_id = -1; // see debug_dump() 'bone id'
    v_Highlight = 0;

    mat4 S = mat4(0.0f);
    for (int i = 0; i < 4; ++i)
    {
        if (in_BoneIds[i] >= 0)
        {
            S += (bone_transforms[in_BoneIds[i]] * in_Weights[i]);
            // Debug.
            if (in_BoneIds[i] == debug_bone_id)
            {
                v_Highlight = 1;
            }
        }
    }
    if (in_BoneIds[0] < 0)
    {
        // In case vertex has no any bone.
        // For debug purpose, make it visible.
        S = mat4(1.0f);
    }
    mat4 MVP = projection * view * model;
    gl_Position = MVP * S * vec4(in_Position, 1.0f);
    v_Position = vec3(model * S * vec4(in_Position, 1.0f));
    v_UV = in_UV;
    
    mat3 MS = mat3(model * S);
    vec3 T = normalize(MS * in_Tangent);
    vec3 B = normalize(MS * in_Bitangent);
    vec3 N = normalize(MS * in_Normal);
    v_TBN = mat3(T, B, N);
    v_Normal = N;
}
)";
        const char* FragmentShader = R"(
        #version 330 core

uniform sampler2D diffuse_sampler;
uniform sampler2D normal_sampler;
uniform vec3 light_position;
uniform vec3 view_position;

// Debug_Flags.x: 1 if need to use per-vertex Normals (v_Normal).
// Debug_Flags.y: unused.
// Debug_Flags.z: unused.
uniform vec3 Debug_Flags;

in vec2 v_UV;
in vec3 v_Position;
in vec3 v_Normal;
in mat3 v_TBN;
// Debug: highlight with green.
flat in int v_Highlight;

out vec4 _Color;

void main()
{
    //view_position = vec3(0.0f, 0.0f, 0.0f);
    if (v_Highlight == 1)
    {
        _Color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        return;
    }

    vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
    float abbient_K = 0.6f;
    float specular_K = 1.2f;
    float specular_P = 100.0f;

    // Ambient.
    vec3 ambient = abbient_K * light_color;
    
    // Diffuse.
    vec3 N = vec3(texture(normal_sampler, v_UV));
    N = N * 2.0 - 1.0;
    N = normalize(v_TBN * N);

    if (Debug_Flags.x > 0)
    {
        N = normalize(v_Normal);
    }

    vec3 light_dir = normalize(light_position - v_Position);
    float diff = max(dot(N, light_dir), 0.0f);
    vec3 diffuse = diff * light_color;
    
    // Specular.
    vec3 view_dir = normalize(view_position - v_Position);
    vec3 reflect_dir = reflect(-light_dir, N);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0f), specular_P);
    vec3 specular = specular_K * spec * light_color;
    
    vec3 object_color = vec3(texture(diffuse_sampler, v_UV));
    vec3 color = (ambient + diffuse + specular) * object_color;
    _Color = vec4(color, 1.0f);
}
)";
        ShaderProgram shader = ShaderProgram::FromBuffers(VertexShader, FragmentShader);
        RenderModel model(std::move(textures), std::move(shader));
        model.meshes = std::move(meshes);
        const unsigned ShaderHandle = model.shader.id;
        glUseProgram(ShaderHandle);
        model.diffusePtr = glGetUniformLocation(ShaderHandle, "diffuse_sampler");
        model.normalPtr = glGetUniformLocation(ShaderHandle, "normal_sampler");
        model.projPtr = glGetUniformLocation(ShaderHandle, "projection");
        model.viewPtr = glGetUniformLocation(ShaderHandle, "view");
        model.modelPtr = glGetUniformLocation(ShaderHandle, "model");
        model.transformPtr = glGetUniformLocation(ShaderHandle, "bone_transforms");
        model.lightPosPtr = glGetUniformLocation(ShaderHandle, "light_position");
        model.viewPosPtr = glGetUniformLocation(ShaderHandle, "view_position");
        model.debugFlagPtr = glGetUniformLocation(ShaderHandle, "Debug_Flags");
        assert(model.diffusePtr >= 0);
        assert(model.normalPtr >= 0);
        assert(model.projPtr >= 0);
        assert(model.viewPtr >= 0);
        assert(model.modelPtr >= 0);
        assert(model.transformPtr >= 0);
        assert(model.lightPosPtr >= 0);
        assert(model.viewPosPtr >= 0);
        assert(model.debugFlagPtr >= 0);

        return model;
    }

void printDrawInputs(std::span<const glm::mat4> transforms, glm::mat4 projection, glm::mat4 view, 
                    glm::mat4 model, glm::vec3 lightPos, glm::vec3 viewPos)
{
    std::cout << "===== DRAW FUNCTION INPUTS =====" << std::endl;
    
    // Print transforms array summary
    std::cout << "Transforms: " << transforms.size() << " matrices" << std::endl;
    if (transforms.size() > 0) {
        std::cout << "First transform[0]:" << std::endl;
        for (int i = 0; i < 4; i++) {
            std::cout << "  [";
            for (int j = 0; j < 4; j++) {
                std::cout << transforms[0][i][j] << (j < 3 ? ", " : "");
            }
            std::cout << "]" << std::endl;
        }
    }
    
    // Print matrices
    std::cout << "Projection Matrix:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  [";
        for (int j = 0; j < 4; j++) {
            std::cout << projection[i][j] << (j < 3 ? ", " : "");
        }
        std::cout << "]" << std::endl;
    }
    
    std::cout << "View Matrix:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  [";
        for (int j = 0; j < 4; j++) {
            std::cout << view[i][j] << (j < 3 ? ", " : "");
        }
        std::cout << "]" << std::endl;
    }
    
    std::cout << "Model Matrix:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  [";
        for (int j = 0; j < 4; j++) {
            std::cout << model[i][j] << (j < 3 ? ", " : "");
        }
        std::cout << "]" << std::endl;
    }
    
    // Print vectors
    std::cout << "Light Position: [" << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << "]" << std::endl;
    std::cout << "View Position: [" << viewPos.x << ", " << viewPos.y << ", " << viewPos.z << "]" << std::endl;
    
    // Print uniform locations
    std::cout << "Uniform Locations:" << std::endl;
    std::cout << "  diffusePtr: " << diffusePtr << std::endl;
    std::cout << "  normalPtr: " << normalPtr << std::endl;
    std::cout << "  projPtr: " << projPtr << std::endl;
    std::cout << "  viewPtr: " << viewPtr << std::endl;
    std::cout << "  modelPtr: " << modelPtr << std::endl;
    std::cout << "  transformPtr: " << transformPtr << std::endl;
    std::cout << "  lightPosPtr: " << lightPosPtr << std::endl;
    std::cout << "  viewPosPtr: " << viewPosPtr << std::endl;
    std::cout << "=================================" << std::endl;
}

    void draw(std::span<const glm::mat4> transforms
        , glm::mat4 projection
        , glm::mat4 view
        , glm::mat4 model
        , glm::vec3 light_position
        , glm::vec3 view_position)
    {
        assert(transforms.size() > 0);
        assert(transforms.size() <= Animation::MaxBones);
        glUseProgram(shader.id);
        glUniformMatrix4fv(projPtr, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewPtr, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelPtr, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(transformPtr, GLsizei(transforms.size()), GL_FALSE, glm::value_ptr(transforms[0]));
        glUniform3fv(lightPosPtr, 1, glm::value_ptr(light_position));
        glUniform3fv(viewPosPtr, 1, glm::value_ptr(view_position));

        for (RenderMesh& mesh : meshes)
        {
            mesh.draw(textures, diffusePtr, normalPtr, debugFlagPtr);
        }
    }


private:

    explicit RenderModel(TexturesDB&& textures, ShaderProgram&& shader)
        : textures(std::move(textures)), shader(std::move(shader)) {}

    TexturesDB textures;
    ShaderProgram shader;
    std::vector<RenderMesh> meshes;
    int diffusePtr = -1;
    int normalPtr = -1;
    int projPtr = -1;
    int viewPtr = -1;
    int modelPtr = -1;
    int transformPtr = -1;
    int lightPosPtr = -1;
    int viewPosPtr = -1;
    int debugFlagPtr = -1;
public:
    static std::vector<RenderMesh> LoadRenderMesh(TexturesDB&& textures, std::vector<AnimMesh>&& animMeshes)
    {
        auto loadByType = [&](AnimMesh& mesh, TextureType type) 
        {
            auto it = std::find_if(mesh.textures.begin(), mesh.textures.end(), [&](const AnimTexture& tex) { return tex.type == type; });
            if(it != mesh.textures.end())
            {
                return textures.add(RenderTexture::LoadTexture(*it));
            }
        };
        std::vector<RenderMesh> renderMeshes;
        for (AnimMesh& mesh : animMeshes)
        {
            renderMeshes.push_back(
                RenderMesh::FromMemory(std::move(mesh.vertices), 
                std::move(mesh.indices), 
                loadByType(mesh, TextureType::Diffuse), 
                loadByType(mesh, TextureType::Normal)));
        }
        return renderMeshes;
    }
};

struct AssimpModel
{
    RenderModel model;
    Animation animation;

    static AssimpModel LoadAnimatedModel(std::filesystem::path path, int animationIndex = -1)
    {
        Assimp::Importer importer;
        (void)importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
        const aiScene* scene = importer.ReadFile(path.string()
        , aiProcess_Triangulate
        | aiProcess_CalcTangentSpace
        | aiProcess_FlipUVs
        | aiProcess_LimitBoneWeights);

        if(!scene)
        {
            std::fprintf(stderr, "Error loading model: %s\n", importer.GetErrorString());
        }
        
        assert((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == 0);
        assert(scene->mRootNode);
        
        BoneInfoRemap bonesInfo;
        TexturesDB textures;
        std::vector<AnimMesh> meshes = AssimpAnimation::LoadModelMeshWithAnimationsWeights(path, *scene, bonesInfo);
        if(!bonesInfo.hasAnyBones())
        {
            std::fprintf(stderr, "Model has no bones. Loading without animation.\n");
        }
        
        Animation anim = AssimpAnimation::LoadAnimation(*scene, animationIndex, bonesInfo);
        std::vector<RenderMesh> renderMeshes = RenderModel::LoadRenderMesh(std::move(textures), std::move(meshes));
        RenderModel model = RenderModel::MakeSimpleNormalMapping(std::move(textures), std::move(renderMeshes));
        return AssimpModel{std::move(model), std::move(anim)};
    }
};

