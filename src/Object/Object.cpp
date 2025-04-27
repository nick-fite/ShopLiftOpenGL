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
        aiString diffuseFile;
        aiString specularFile;
        aiString normalFile;
        aiString BaseColorFile;
        aiString normalCameraFile;

        material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseFile);
        material->GetTexture(aiTextureType_SPECULAR, 0, &specularFile);
        material->GetTexture(aiTextureType_NORMALS, 0, &normalFile);
        material->GetTexture(aiTextureType_BASE_COLOR, 0, &BaseColorFile);
        material->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &normalCameraFile);

        std::cout << "diffuse: " << diffuseFile.C_Str() << std::endl;

        //sets up material
        Shader* vertShader = new Shader("../../assets/Shaders/Vertex.glsl", GL_VERTEX_SHADER);
        Shader* fragShader = new Shader("../../assets/Shaders/Fragment.glsl", GL_FRAGMENT_SHADER);
        ShaderProgram* shaderProgram = new ShaderProgram();
        shaderProgram->AttachShader(vertShader);
        shaderProgram->AttachShader(fragShader);
        Material* mat = new Material(shaderProgram);
        
        if(scene->mNumTextures > 0)
        {
            for(int j = 0; j < scene->mNumTextures; j++)
            {
                std::string temp = scene->mTextures[j]->mFilename.C_Str();
                std::string diffuseTemp = diffuseFile.C_Str();
                std::string specularTemp = specularFile.C_Str();
                std::string normalTemp = normalFile.C_Str();
                std::string baseColorTemp = BaseColorFile.C_Str();
                std::string normalCameraTemp = normalCameraFile.C_Str();
                if(temp == diffuseTemp || temp == specularTemp || temp == normalTemp || temp == baseColorTemp || temp == normalCameraTemp)
                {
                    std::string textureName = scene->mTextures[j]->mFilename.C_Str();
                    std::cout << "texture name: " << textureName << std::endl;
                    if(textureName.find("diffuse") != std::string::npos)
                    {
                        std::cout << "diffuse texture: " << textureName << std::endl;
                        Texture* tex = CreateTextureFromEmbedded(scene->mTextures[j]);
                        mat->SetTexture("texDiffuse", tex);
                    }
                    else if(textureName.find("specular") != std::string::npos)
                    {
                        std::cout << "specular texture: " << textureName << std::endl;
                        Texture* tex = CreateTextureFromEmbedded(scene->mTextures[j]);
                        mat->SetTexture("texSpecular", tex);
                    }
                    else if(textureName.find("normal") != std::string::npos)
                    {
                        std::cout << "normal texture: " << textureName << std::endl;
                        Texture* tex = CreateTextureFromEmbedded(scene->mTextures[j]);
                        mat->SetTexture("texNormal", tex);
                    }
                    else if(textureName.find("BaseColor") != std::string::npos)
                    {
                        std::cout << "BaseColor texture: " << textureName << std::endl;
                        Texture* tex = CreateTextureFromEmbedded(scene->mTextures[j]);
                        mat->SetTexture("texDiffuse", tex);
                    }
                    else if(textureName.find("height") != std::string::npos)
                    {
                        std::cout << "height texture: " << textureName << std::endl;
                    }
                }
            }
        }
        else 
        {
            std::cout << "no textures found" << std::endl;
            Texture* tex = new Texture("../../assets/TestAssets/Textures/Solid_gray.png");
            mat->SetTexture("texDiffuse", tex);
            mat->SetTexture("texSpecular", tex);
            mat->SetTexture("texNormal", tex);
        }

        
        
        Mesh* newMesh = new Mesh(assimpMesh);
        MeshInfo newMeshInfo;
        newMeshInfo.name = "name";
        newMeshInfo.mesh = newMesh;
        newMeshInfo.material = mat;
        Meshes.push_back(newMeshInfo);
        ExtractBoneWeightForverticies(newMesh->GetMeshData(), assimpMesh, scene);
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
void Object::DrawMeshes(glm::mat4 viewProjection, glm::mat4 transformMatrix, glm::vec3 cameraPos)
{
    for(int i = 0; i < Meshes.size(); i++)
    {
        //std::cout << "drawing: " << Meshes[i].name << std::endl;
        Meshes[i].material->SetMatrix("cameraView", viewProjection);
        Meshes[i].material->SetMatrix("worldMatrix", transformMatrix);

        GLuint shaderProgram = Meshes[i].material->GetShaderProgram()->GetGLShaderProgram();
        GLint viewPosLocation = glGetUniformLocation(shaderProgram, "viewPos");
        if (viewPosLocation != -1) {
            glUniform3fv(viewPosLocation, 1, &cameraPos[0]);
        }

        Meshes[i].material->Bind();
        Meshes[i].mesh->DrawMesh();
        Meshes[i].material->UnBind();
    }
}


void Object::SetVertexBoneData(MeshData& meshData, int boneID, float weight)
{
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(meshData.m_BoneIDs[i] < 0)
        {
            meshData.m_BoneIDs[i] = boneID;
            meshData.m_Weights[i] = weight;
            break;
        }
    }   
}
void Object::ExtractBoneWeightForverticies(std::vector<MeshData>& verticies, aiMesh* mesh, const aiScene* scene)
{
    std::cout << "extracting bone weights" << std::endl;
    auto& boneInfoMap = m_BoneMap;
    int& boneCount = m_boneCount;

    for(int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if(boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo* newBoneInfo = new BoneInfo();
            newBoneInfo->ID = boneCount;
            newBoneInfo->offsetMatrix = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = boneInfoMap[boneName]->ID;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for(int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexID = weights[weightIndex].mVertexId;
            float weightValue = weights[weightIndex].mWeight;
            assert(vertexID <= verticies.size());
            SetVertexBoneData(verticies[vertexID], boneID, weightValue);
        }

    }
    m_BoneMap = boneInfoMap;
    m_boneCount = boneCount;
}
    
void Object::SetVertexBoneDataToDefault(MeshData& meshData) 
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		meshData.m_BoneIDs[i] = -1;
		meshData.m_Weights[i] = 0.0f;
	}
}