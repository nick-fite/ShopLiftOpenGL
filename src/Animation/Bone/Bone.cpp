#include "Bone.h"
Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
: m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
{
    m_NumPositions = channel->mNumPositionKeys;

    for(int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
    {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = GetGLMVec(aiPosition);
        data.timeStamp = timeStamp;
        m_positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for(int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
    {
        aiQuaternion aiRotation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.rotation = GetGLMQuat(aiRotation);
        data.timeStamp = timeStamp;
        m_rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;

    for(int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
    {
        aiVector3D aiScale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        KeyScale data;
        data.scale = GetGLMVec(aiScale);
        data.timeStamp = timeStamp;
        m_scales.push_back(data);
    }
}

void Bone::Update(float animationTime)
{
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);

    m_LocalTransform = translation * rotation * scale;
}

glm::mat4 Bone::GetLocalTransform() const
{
    return m_LocalTransform;
}

std::string Bone::GetBoneName() const
{
    return m_Name;
}

int Bone::GetBoneID() const
{
    return m_ID;
}

int Bone::GetPositionIndex(float AnimationTime) const
{
    for(int index = 0; index < m_NumPositions - 1; ++index)
    {
        if(AnimationTime < m_positions[index + 1].timeStamp)
        {
            return index;
        }
    }
    assert(0);
    return 0;
}

int Bone::GetRotationIndex(float AnimationTime) const
{
    for(int index = 0; index < m_NumRotations - 1; ++index)
    {
        if(AnimationTime < m_rotations[index + 1].timeStamp)
        {
            return index;
        }
    }
    assert(0);
    return 0;
}

int Bone::GetScaleIndex(float AnimationTime) const
{
    for(int index = 0; index < m_NumScalings - 1; ++index)
    {
        if(AnimationTime < m_scales[index + 1].timeStamp)
        {
            return index;
        }
    }
    assert(0);
    return 0;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float AnimationTime) const
{
    float scaleFactor = 0.0f;
    float midWayLength = AnimationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::InterpolatePosition(float animationTime) const
{
    if(1 == m_NumPositions)
    {
        return glm::translate(glm::mat4(1.0f), m_positions[0].position);
    }
    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(m_positions[p0Index].timeStamp, m_positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_positions[p0Index].position, m_positions[p1Index].position, scaleFactor);

    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime) const
{
    if(1 == m_NumRotations)
    {
        return glm::mat4_cast(m_rotations[0].rotation);
    }
    int r0Index = GetRotationIndex(animationTime);
    int r1Index = r0Index + 1;

    float scaleFactor = GetScaleFactor(m_rotations[r0Index].timeStamp, m_rotations[r1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[r0Index].rotation, m_rotations[r1Index].rotation, scaleFactor);

    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::InterpolateScaling(float AnimationTime) const
{
    if(1 == m_NumScalings)
    {
        return glm::scale(glm::mat4(1.0f), m_scales[0].scale);
    }
    int s0Index = GetScaleIndex(AnimationTime);
    int s1Index = s0Index + 1;

    float scaleFactor = GetScaleFactor(m_scales[s0Index].timeStamp, m_scales[s1Index].timeStamp, AnimationTime);
    glm::vec3 finalScale = glm::mix(m_scales[s0Index].scale, m_scales[s1Index].scale, scaleFactor);

    return glm::scale(glm::mat4(1.0f), finalScale);
}
