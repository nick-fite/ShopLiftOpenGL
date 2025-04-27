#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include "../Object/Object.h"
#include "Bone/Bone.h"
#include "../BoneInfo.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

struct AssimpNodeData
{
    std::string name;
    glm::mat4 transform;
    int childrenCount = 0;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;
    Animation(const std::string& animationPath, Object* obj);
    ~Animation(){};

    Bone* FindBone(const std::string& name) const;
    inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
    inline float GetDuration() const { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    inline const std::map<std::string, BoneInfo*>& GetBoneMap() const { return m_BoneMap; }

private:
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
    void ReadMissingBones(const aiAnimation* animation, Object* obj);

    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo*> m_BoneMap;
    std::vector<Bone*> m_Bones;
    float m_Duration = 0.0f;
    float m_TicksPerSecond = 0.0f;

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


};