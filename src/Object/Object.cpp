#include "Object.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
Object::Object(std::string filePath, std::string name)
{
    std::ifstream file("../../assets/TestAssets/SpaceGuy.fbx");
    
    if(!file.good())
    {
        std::cout << "what the fuc\n"<< "bad file: " << filePath << std::endl;
        return;
    }

    Assimp::Importer importer;
    const aiScene* scene = NULL;

    for(int k = 0; k < 1; k++)
    {
        scene = importer.ReadFile("../../assets/TestAssets/TestPlayer.fbx", aiProcessPreset_TargetRealtime_Quality | aiProcess_PreTransformVertices);
    }

    if(scene)
    {
        std::cout << "Scene loaded" << std::endl;
    }
    else
    {
        std::cout << "FAILURE Scene not loaded" << std::endl;
    }

    for(int i = 0; i < scene->mNumMeshes; i++)
    {
        std::string newName = name + "_" + char(i);
        std::cout << newName << std::endl;
        meshMap.insert({newName, new Mesh(scene->mMeshes[i])});
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
