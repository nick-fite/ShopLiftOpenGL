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

    for(int i = 0; i < scene->mNumMeshes; i++)
    {
        std::string newName = name + "_" + char(i);
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
