#include "Animation.h"
Animation::Animation(glm::mat4 rootInverse, std::vector<animNode>&& nodes, unsigned bones_count, float duration, float ticksPerSecond)
: globalRootInverse(rootInverse), 
  matrixTransforms(MaxBones, glm::mat4(1.0f)), 
  nodes(std::move(nodes)), 
  bonesCount(bones_count), 
  duration(duration), 
  ticksPerSecond(ticksPerSecond)
{

}

void Animation::update(float dt)
{
    currentTime += ticksPerSecond * dt;
    currentTime = fmod(currentTime, duration);
    for(std::size_t i = 0; i < nodes.size(); ++i)
    {
        animNode& node = nodes[i];
        assert(int(i) > node.parent);

        const glm::mat4 transform = (node.boneKeyFrames && node.boneKeyFrames->hasAnyKeyframes()) ? 
        node.boneKeyFrames->InterpolateFramesAt(currentTime) : node.localTransform;
        
        const glm::mat4 parentTransform = (node.parent >= 0) ? 
        nodes[node.parent].transform : glm::mat4(1.0f);

        node.transform = parentTransform * transform;
        if(!node.boneKeyFrames)
        {
            continue;
        }

        const std::size_t boneIndex = node.boneKeyFrames->boneIndex;
        assert(boneIndex < matrixTransforms.size() && "Too many bones, see bone limit.");

        matrixTransforms[boneIndex] = globalRootInverse * node.transform * node.boneKeyFrames->inverseBindPose;
    }

}

std::span<const glm::mat4> Animation::transforms() const
{
    const glm::mat4* ptr = matrixTransforms.data();
    const std::size_t count = bonesCount > 0 ? std::size_t(bonesCount) : matrixTransforms.size();
    return std::span<const glm::mat4>(ptr, count);
}

void Animation::debugDump() const
{

}