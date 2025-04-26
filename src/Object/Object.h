#include "../mesh/Mesh.h"
#include "../Material/Material.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <filesystem>

#define ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices |    \
                           aiProcess_Triangulate |              \
                           aiProcess_GenSmoothNormals |         \
                           aiProcess_LimitBoneWeights |         \
                           aiProcess_SplitLargeMeshes |         \
                           aiProcess_ImproveCacheLocality |     \
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
    struct MeshInfo {
        std::string name;
        Mesh* mesh;
        Material* material;
    };

    std::vector<MeshInfo> Meshes;
    //std::vector<Mesh*> meshes;

    void ProcessNode(aiNode* node, const aiScene* scene, std::string filePath);

    Texture* CreateTextureFromEmbedded(const aiTexture* embeddedTexture) {
    // Check if the texture is compressed (stored in a format like PNG, JPG)
    if (embeddedTexture->mHeight == 0) {
        // Compressed texture - data is stored as a blob in mData
        std::string formatHint = embeddedTexture->achFormatHint;
        std::cout << "Embedded compressed texture found with format hint: " << formatHint << std::endl;
        
        // Create a temporary file to store the texture data
        std::string tempFileName = "temp_embedded_texture." + formatHint;
        std::ofstream outFile(tempFileName, std::ios::binary);
        
        if (outFile.is_open()) {
            outFile.write(reinterpret_cast<const char*>(embeddedTexture->pcData), 
                         embeddedTexture->mWidth); // mWidth contains size in bytes
            outFile.close();
            
            char pathBuffer[512];
            strncpy(pathBuffer, tempFileName.c_str(), sizeof(pathBuffer));
            pathBuffer[sizeof(pathBuffer) - 1] = '\0';
            // Create a texture from the temporary file
            Texture* texture = new Texture(pathBuffer);
            
            // Remove the temporary file
            std::remove(tempFileName.c_str());
            
            return texture;
        }
    } else {
        // Uncompressed texture - data is stored as ARGB8888 pixels
        std::cout << "Embedded uncompressed texture found: " << 
            embeddedTexture->mWidth << "x" << embeddedTexture->mHeight << std::endl;
        
        // Create the texture directly from pixel data
        // This would require a custom constructor in your Texture class
        // Texture* texture = new Texture(embeddedTexture->pcData, 
        //                               embeddedTexture->mWidth, 
        //                               embeddedTexture->mHeight);
        // return texture;
    }
    
    return nullptr; // Return nullptr if texture creation failed
}

    void SceneNodeNames(const aiScene* scene)
    {
        std::cout << std::endl;
        NodeRecurssive(scene->mRootNode);
    }

    void NodeRecurssive(aiNode* node)
    {
        for(int i = 0; i < node->mNumChildren; i++)
        {
            std::cout << "Node name: " << node->mChildren[i]->mName.C_Str() << std::endl;
            NodeRecurssive(node->mChildren[i]);
        }
    }

public:
    Object(std::string filePath, std::string name, bool loadSkeleton = false);
    ~Object();
    std::string name;

    void DrawMeshes(glm::mat4 viewProjection, glm::mat4 transformMatrix, glm::vec3 cameraPos);
};