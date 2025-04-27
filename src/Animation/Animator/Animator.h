#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "../Animation.h"
#include "../Bone/Bone.h"

class Animator {
public:
    Animator(Animation* anim);
    void UpdateAnimation(float dt);
    void PlayAnimation(Animation* pAnimation);
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    std::vector<glm::mat4> GetFinalBoneMatrices() const;
private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_currentTime;
    float m_DeltaTime;
};