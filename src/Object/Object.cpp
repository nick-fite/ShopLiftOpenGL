#include "Object.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <filesystem>

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
            scene = importer.ReadFile(filePath, aiProcessPreset_TargetRealtime_Quality | aiProcess_PreTransformVertices);
        }
    }

    ProcessNode(scene->mRootNode, scene, filePath);
    int num = scene->mNumTextures;
    std::cout << "scene textures: " << num;
}
void Object::ProcessNode(aiNode* node, const aiScene* scene, std::string filePath)
{
    //std::filesystem::path modelPath(filePath);
    //std::filesystem::path modelDir = modelPath.parent_path();
    
    for(unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        std::string newName = name + "_" + char(i);
        
        aiMesh* assimpMesh = scene->mMeshes[i];
        int matIndex = assimpMesh->mMaterialIndex;
        aiMaterial* material = scene->mMaterials[matIndex];
        //aiString diffuseFile;
        //aiString specularFile;
        //aiString normalFile;

        //get's relative texture path
        //material->GetTexture((aiTextureType)1, 0, &diffuseFile);
        //material->GetTexture((aiTextureType)2, 0, &specularFile);
        //material->GetTexture((aiTextureType)6, 0, &normalFile);        

        //std::cout << "material name: " << diffuseFile.data << std::endl;

        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; type++) {
            unsigned int count = material->GetTextureCount((aiTextureType)type);
    
            for(int i = 0; i < count; i++)
            {
                aiString str;
                material->GetTexture((aiTextureType)type, i, &str);

                //std::string path = str.C_Str();
                //std::cout << path << std::endl;
                
            }


            if (count > 0) {
                //std::cout << "Texture type " << type << " has " << count << " textures.\n";
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
        
        if(scene->mNumTextures > 0)
        {
            Texture* texDiffuse = CreateTextureFromEmbedded(scene->mTextures[0]);
            //Texture* texNormal = CreateTextureFromEmbedded(scene->mTextures[1]);
            //Texture* texSpecular = CreateTextureFromEmbedded(scene->mTextures[2]);
            mat->SetTexture("tex", texDiffuse);
            //mat->SetTexture("texSpecular", texSpecular);
            //mat->SetTexture("texNormal", texNormal);
        }
        else 
        {
            Texture* texDiffuse = new Texture("../../assets/TestAssets/Textures/Solid_gray.png");
            //Texture* texNormal = new Texture("../../assets/TestAssets/Textures/Solid_gray.png");
            //Texture* texSpecular = new Texture("../../assets/TestAssets/Textures/Solid_gray.png");
            mat->SetTexture("tex", texDiffuse);
            //mat->SetTexture("texSpecular", texSpecular);
            //mat->SetTexture("texNormal", texNormal);
        }

        
        Mesh* newMesh = new Mesh(assimpMesh);
        MeshInfo newMeshInfo;
        newMeshInfo.name = "name";
        newMeshInfo.mesh = newMesh;
        newMeshInfo.material = mat;
        Meshes.push_back(newMeshInfo);
    }
    
    //for(unsigned int i = 0; i < node->mNumChildren; i++)
   // {
   //     ProcessNode(node->mChildren[i], scene, filePath);
    //}
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
