#include "../mesh/Mesh.h"
#include "../Material/Material.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <filesystem>
#include "../BoneInfo.h"

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

#define ASSIMP_LOAD_SKELETON_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace)

class Object {
private:
    typedef std::map<std::string, std::map<Mesh*, Material*>> meshMapType;
    struct MeshInfo {
        std::string name;
        Mesh* mesh;
        Material* material;
    };

    std::vector<MeshInfo> Meshes;
    std::map<std::string, BoneInfo*> m_BoneMap;
    int m_boneCount = 0;
    

    void ProcessNode(aiNode* node, const aiScene* scene, std::string filePath);

    void SetVertexBoneData(MeshData& meshData, int boneID, float weight);
    void ExtractBoneWeightForverticies(std::vector<MeshData>& verticies, aiMesh* mesh, const aiScene* scene);
    void SetVertexBoneDataToDefault(MeshData& meshData);
    
    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}
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

        std::cout << "something went wrong with the texture" << std::endl;
        return nullptr; // Return nullptr if texture creation failed
    }
public:
    Object(std::string filePath, std::string name, bool loadSkeleton = false);
    ~Object();
    std::string name;
    Material* mat;

    void DrawMeshes(glm::mat4 viewProjection, glm::mat4 transformMatrix, glm::vec3 cameraPos);

    std::map<std::string, BoneInfo*>& GetBoneInfoMap() {
        return m_BoneMap;
    }
    int& getBoneCount() {
        return m_boneCount;
    }
};