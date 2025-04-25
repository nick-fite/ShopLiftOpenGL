#include "Object.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
Object::Object(std::string filePath, std::string name, bool loadSkeleton)
{
    std::ifstream file(filePath);
    
    if(!file.good())
    {
        std::cout << "what the fuc\n"<< "bad file: " << filePath << std::endl;
        return;
    }

    Assimp::Importer importer;
    const aiScene* scene = NULL;

    for(int k = 0; k < 1; k++)
    {
        if(loadSkeleton)
        {
            scene = importer.ReadFile(filePath, ASSIMP_LOAD_SKELETON_FLAGS);
        }
        else
        {
            scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_ValidateDataStructure);
        }
    }

    if(scene)
    {
        std::cout << "Scene loaded" << std::endl;
    }
    else
    {
        std::cout << "FAILURE Scene not loaded" << std::endl;
    }

    std::cout << "skeleton: " << scene->hasSkeletons() << std::endl;


    ProcessNode(scene->mRootNode, scene);
    int num = scene->mNumTextures;
    std::cout << "scene textures: " << num;
}
void Object::ProcessNode(aiNode* node, const aiScene* scene)
{
    
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        std::string newName = name + "_" + char(i);
        
        aiMesh* assimpMesh = scene->mMeshes[i];
        int matIndex = assimpMesh->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        aiString textureFile;
        material->GetTexture((aiTextureType)1, i, &textureFile);
        
        std::cout << "material name: " << material->GetName().C_Str() << std::endl;

        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; type++) {
            unsigned int count = material->GetTextureCount((aiTextureType)type);
    
            for(int i = 0; i < count; i++)
            {
                aiString str;
                material->GetTexture((aiTextureType)type, i, &str);

                std::string path = str.C_Str();
                std::cout << path << std::endl;
                
            }


            if (count > 0) {
                std::cout << "Texture type " << type << " has " << count << " textures.\n";
            }
        }

        //sets up material
        Shader* vertShader = new Shader("../../assets/Shaders/Vertex.glsl", GL_VERTEX_SHADER);
        Shader* fragShader = new Shader("../../assets/Shaders/Fragment.glsl", GL_FRAGMENT_SHADER);
        char textureFS[] = "tex";
        
        ShaderProgram* shaderProgram = new ShaderProgram();
        shaderProgram->AttachShader(vertShader);
        shaderProgram->AttachShader(fragShader);
        Material* mat = new Material(shaderProgram);

        std::cout << "texture: " << textureFile.data << std::endl;
        Texture* tex = new Texture(textureFile.data);
        tex->GetGLTexture();
        mat->SetTexture(textureFS, tex);
        
        Mesh* newMesh = new Mesh(assimpMesh);
        MeshInfo newMeshInfo;
        newMeshInfo.name = "name";
        newMeshInfo.mesh = newMesh;
        newMeshInfo.material = mat;
        Meshes.push_back(newMeshInfo);
    }
    
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Object::~Object()
{
    for(int i = 0; i < Meshes.size(); i++)
    {
        delete Meshes[i].mesh;
        delete Meshes[i].material;
    }

}
void Object::DrawMeshes(glm::mat4 viewProjection, glm::mat4 transformMatrix)
{
    for(int i = 0; i < Meshes.size(); i++)
    {
        //std::cout << "drawing: " << Meshes[i].name << std::endl;
        Meshes[i].material->SetMatrix("cameraView", viewProjection);
        Meshes[i].material->SetMatrix("worldMatrix", transformMatrix);
        Meshes[i].material->Bind();
        Meshes[i].mesh->DrawMesh();
        Meshes[i].material->UnBind();
    }
}
