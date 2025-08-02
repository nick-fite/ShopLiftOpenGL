#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <filesystem>
#include <map>
#include "../animation/Animation.h"

using BoneIndex = int;

static glm::mat4 Matrix_RowToColumn(const aiMatrix4x4& m)
{
    const glm::vec4 c1(m.a1, m.b1, m.c1, m.d1);
    const glm::vec4 c2(m.a2, m.b2, m.c2, m.d2);
    const glm::vec4 c3(m.a3, m.b3, m.c3, m.d3);
    const glm::vec4 c4(m.a4, m.b4, m.c4, m.d4);
    return glm::mat4(c1, c2, c3, c4);
}

static glm::vec3 Vec_ToGLM(const aiVector3D& vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

static glm::quat Quat_ToGLM(const aiQuaternion& pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

enum class TextureType
{
    None,
    Diffuse,
    Normal
};

struct MemoryTexture
{
    std::vector<std::uint8_t> png;
};

struct AnimTexture 
{
    MemoryTexture memoryTexture;
    std::filesystem::path path;
    TextureType type = TextureType::None;
};

struct AnimMesh 
{
    std::vector<AnimVertex> vertices;
    std::vector<unsigned> indices;
    std::vector<AnimTexture> textures;
};

struct BoneMeshInfo
{
    BoneIndex index = -1;
    glm::mat4 inverseBindPose;
};

struct BoneInfoRemap
{
    std::map<std::string, BoneMeshInfo, std::less<>> nameToInfo;
    boneIndex nextBoneId = 0;

    std::map<BoneIndex, int> debugBoneVertices;

    BoneIndex AddNewBone(const std::string& name, const glm::mat4& inverseBindPose)
    {
       auto [it, inserted] = nameToInfo.insert(std::make_pair(std::move(name), BoneMeshInfo{}));
       if(inserted)
       {
            BoneMeshInfo& info = it->second;
            info.index = nextBoneId++;
            info.inverseBindPose = inverseBindPose;
            return info.index;
       }
       else
       {
            const BoneMeshInfo& oldInfo = it->second;
            assert(oldInfo.inverseBindPose == inverseBindPose);
            return oldInfo.index;
        }
    }

    const BoneMeshInfo* FindBone(const std::string& name) const
    {
        auto it = nameToInfo.find(name);
        if(it != nameToInfo.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    bool hasAnyBones() const
    {
        return nameToInfo.size() > 0;
    }

    void RecordVertex(BoneIndex boneIndex)
    {
        assert(boneIndex >= 0);
        ++debugBoneVertices[boneIndex];   
    }

    int verticesCount(BoneIndex boneIndex)const 
    {
        auto it = debugBoneVertices.find(boneIndex);
        return ((it != debugBoneVertices.end()) ? it->second : -1);
    }
};

class AssimpAnimation {
public:
    static AnimMesh LoadMesh(const std::filesystem::path& path, const aiMesh& mesh, const aiScene& scene, BoneInfoRemap& boneInfoRemap);
    static std::vector<AnimMesh> LoadModelMeshWithAnimationsWeights(const std::filesystem::path& modelPath, 
        const aiScene& scene, BoneInfoRemap& boneInfo);
    static BoneKeyFrames LoadBoneKeyFrames(const aiNodeAnim& channel, const BoneMeshInfo& boneMeshInfo);
    static Animation LoadAnimation(const aiScene& scene, int animationIndex, const BoneInfoRemap& boneInfoRemap);

};