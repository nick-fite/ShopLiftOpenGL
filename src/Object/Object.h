#include "../mesh/Mesh.h"
#include "../Material/Material.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>

#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_CalcTangentSpace|          \
                           aiProcess_PreTransformVertices)

#define ASSIMP_LOAD_SKELETON_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
                           aiProcess_RemoveRedundantMaterials | \
                           aiProcess_FindDegenerates |          \
                           aiProcess_FindInvalidData |          \
                           aiProcess_GenUVCoords |              \
                           aiProcess_CalcTangentSpace)

class Object {

private:
    typedef std::map<std::string, std::map<Mesh*, Material*>> meshMapType;
    //std::vector<Mesh*> meshes;
    
    meshMapType meshes;
    std::map<std::string, Mesh*> meshMap;
public:
    Object(std::string filePath, std::string name, bool loadSkeleton = false);
    ~Object();
    std::string name;

    void DrawMeshes();
};