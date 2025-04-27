#pragma once

#include <vector>
#include <assimp/scene.h>
#include <list>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/quaternion.hpp>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat rotation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);
    void Update(float animationTime);
    glm::mat4 GetLocalTransform() const;
    std::string GetBoneName() const;
    int GetBoneID() const;
    int GetPositionIndex(float AnimationTime) const;
    int GetRotationIndex(float AnimationTime) const;
    int GetScaleIndex(float AnimationTime) const;

private:
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float AnimationTime) const;
    glm::mat4 InterpolatePosition(float animationTime) const;
    glm::mat4 InterpolateRotation(float animationTime) const;
    glm::mat4 InterpolateScaling(float animationTime) const;

    std::vector<KeyPosition> m_positions;
    std::vector<KeyRotation> m_rotations;
    std::vector<KeyScale> m_scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

    static inline glm::vec3 GetGLMVec(const aiVector3D& vec) 
	{ 
		return glm::vec3(vec.x, vec.y, vec.z); 
	}

    static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
	{
		return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
	}
};