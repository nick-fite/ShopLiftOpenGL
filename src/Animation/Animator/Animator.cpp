#include "Animator.h"

Animator::Animator(Animation* anim)
{    
    m_currentTime = 0.0f;
    m_CurrentAnimation = anim;
    m_FinalBoneMatrices.resize(100);
    for(int i = 0; i < 100; i++)
    {
        m_FinalBoneMatrices[i] = glm::mat4(1.0f);
    }
}    
     
void Animator::UpdateAnimation(float dt)
{
    m_DeltaTime = dt;
    if(m_CurrentAnimation)
    {
        m_currentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
        m_currentTime = fmod(m_currentTime, m_CurrentAnimation->GetDuration());
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
    }

}

void Animator::PlayAnimation(Animation* pAnimation)
{
    m_CurrentAnimation = pAnimation;
    m_currentTime = 0.0f;   
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transform;

    Bone* bone = m_CurrentAnimation->FindBone(nodeName);

    if(bone)
    {
        bone->Update(m_currentTime);
        nodeTransform = bone->GetLocalTransform() * nodeTransform;
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->GetBoneMap();

    if(boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName]->ID;
        glm::mat4 offset = boneInfoMap[nodeName]->offsetMatrix;
        m_FinalBoneMatrices[index] = globalTransform * boneInfoMap[nodeName]->offsetMatrix;
    }

    for(int i = 0; i < node->childrenCount; i++)
    {
        CalculateBoneTransform(&node->children[i], globalTransform);
    }
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices() const
{
    return m_FinalBoneMatrices;
}
