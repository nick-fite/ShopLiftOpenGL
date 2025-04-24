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
            scene = importer.ReadFile(filePath, ASSIMP_LOAD_FLAGS);
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

    Shader* vertShader = new Shader("../../assets/Shaders/Vertex.glsl", GL_VERTEX_SHADER);
    Shader* fragShader = new Shader("../../assets/Shaders/Fragment.glsl", GL_FRAGMENT_SHADER);
    char textureFS[] = "tex";
    
    for(int i = 0; i < scene->mNumMeshes; i++)
    {
        std::string newName = name + "_" + char(i);
        
        aiMesh* assimpMesh = scene->mMeshes[i];
        int matIndex = assimpMesh->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        aiString textureFile;
        std::cout <<"Texture none: " << material->GetTextureCount(aiTextureType_NONE) << std::endl;
        std::cout <<"Texture diffuse: " << material->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
        std::cout <<"Texture specular: " << material->GetTextureCount(aiTextureType_SPECULAR) << std::endl;
        std::cout <<"Texture ambient: " << material->GetTextureCount(aiTextureType_AMBIENT) << std::endl;
        std::cout <<"Texture emissive: " << material->GetTextureCount(aiTextureType_EMISSIVE) << std::endl;
        std::cout <<"Texture height: " << material->GetTextureCount(aiTextureType_HEIGHT) << std::endl;
        std::cout <<"Texture normals: " << material->GetTextureCount(aiTextureType_NORMALS) << std::endl;
        std::cout <<"Texture shininess: " << material->GetTextureCount(aiTextureType_SHININESS) << std::endl;
        std::cout <<"Texture opacity: " << material->GetTextureCount(aiTextureType_OPACITY) << std::endl;
        std::cout <<"Texture displacement: " << material->GetTextureCount(aiTextureType_DISPLACEMENT) << std::endl;
        std::cout <<"Texture lightmap: " << material->GetTextureCount(aiTextureType_LIGHTMAP) << std::endl;
        std::cout <<"Texture reflection: " << material->GetTextureCount(aiTextureType_REFLECTION) << std::endl;
        std::cout <<"Texture basecolor: " << material->GetTextureCount(aiTextureType_BASE_COLOR) << std::endl;
        std::cout <<"Texture normal camera: " << material->GetTextureCount(aiTextureType_NORMAL_CAMERA) << std::endl;
        std::cout <<"Texture emission color: " << material->GetTextureCount(aiTextureType_EMISSION_COLOR) << std::endl;
        std::cout <<"Texture metalness: " << material->GetTextureCount(aiTextureType_METALNESS) << std::endl;
        std::cout <<"Texture roughness: " << material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) << std::endl;
        std::cout <<"Texture ambient occlusion: " << material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) << std::endl;
        std::cout <<"Texture unknown: " << material->GetTextureCount(aiTextureType_UNKNOWN) << std::endl;
        std::cout <<"Texture sheen: " << material->GetTextureCount(aiTextureType_SHEEN) << std::endl;
        std::cout <<"Texture clearcoat: " << material->GetTextureCount(aiTextureType_CLEARCOAT) << std::endl;
        std::cout <<"Texture transmission: " << material->GetTextureCount(aiTextureType_TRANSMISSION) << std::endl;
        std::cout <<"Texture maya base: " << material->GetTextureCount(aiTextureType_MAYA_BASE) << std::endl;
        std::cout <<"Texture maya specular: " << material->GetTextureCount(aiTextureType_MAYA_SPECULAR) << std::endl;
        std::cout <<"Texture maya specular color: " << material->GetTextureCount(aiTextureType_MAYA_SPECULAR_COLOR) << std::endl;
        std::cout <<"Texture maya specular roughness: " << material->GetTextureCount(aiTextureType_MAYA_SPECULAR_ROUGHNESS) << std::endl;
     
     
     
     
     

        material->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), textureFile);
        
        if(const aiTexture* texture = scene->GetEmbeddedTexture(textureFile.C_Str()))
        {
            std::cout << "textures found" << std::endl;
        }


        
        //sets up material
        ShaderProgram* shaderProgram = new ShaderProgram();
        shaderProgram->AttachShader(vertShader);
        shaderProgram->AttachShader(fragShader);
        Material* mat = new Material(shaderProgram);
        

        mat->SetTexture(textureFS, new Texture(textureFile.data));
        
        Mesh* newMesh = new Mesh(assimpMesh);
        //Meshes.insert({newName, newMesh, material});
        //meshMap.insert({newName, newMesh});
        
    }
}

Object::~Object()
{
    for(auto i = meshMap.begin(); i != meshMap.end(); i++)
    {
        delete i->second;
    }

}
void Object::DrawMeshes()
{
    for(auto i = meshMap.begin(); i != meshMap.end(); i++)
    {
        i->second->DrawMesh();
    }
}
