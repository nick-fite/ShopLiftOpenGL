#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <optional>
#include <string>
#include <span>
using boneIndex = int;

#define MAX_BONE_INFLUENCE 4

#pragma region HelperStructs

struct AnimVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    boneIndex m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

struct BoneKeyFrames 
{
    boneIndex boneIndex = -1;
    glm::mat4 inverseBindPose = glm::mat4(1.0f);
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;

    bool hasAnyKeyframes() const
    {
        return !positions.empty() && !rotations.empty() && !scales.empty();
    }

    float prevAnimationTime = -1;
    int prevPositionIndex = -1;
    int prevRotationIndex = -1;
    int prevScaleIndex = -1;

    
    glm::mat4 InterpolateFramesAt(float animation_time)
    {
        //std::fprintf(stderr, "InterpolateFramesAt: %f\n", animation_time);
        const glm::mat4 translation = interpolate_position(animation_time);
        const glm::mat4 rotation = interpolate_rotation(animation_time);
        const glm::mat4 scale = interpolat_scaling(animation_time);
        prevAnimationTime = animation_time;
        return translation * rotation * scale;
    }

    template<typename T>
    struct KeyTimeCompare
    {
        bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs.timeStamp < rhs.timeStamp;
        }
        bool operator()(float animationTime, const T& rhs) const
        {
            return animationTime  < rhs.timeStamp;
        }
        bool operator()(const T& lhs, float animationTime) const
        {
            return lhs.timeStamp < animationTime;
        }
    };

    template<typename T>
    static int GetFrameIndex(const std::vector<T>& keyFrames, float animationTime, unsigned startOffset, unsigned endOffset )
    {
        assert(keyFrames.size() >= 2);
        auto it = std::lower_bound(keyFrames.cbegin(), keyFrames.end(), animationTime, KeyTimeCompare<T>{});

        if(it == keyFrames.cbegin())
        {
            it = keyFrames.cbegin() + 1;
        }

        assert(it != keyFrames.cend());
        const int index = (int(std::distance(keyFrames.cbegin(), it)) - 1);
        assert(index >= 0);
        assert(index < (int(keyFrames.size()) - 1));
        //assert(keyFrames[index].timeStamp <= animationTime);
        //assert(keyFrames[index + 1].timeStamp >= animationTime);
        return index;
    };

    template<typename T>
    static int UpdateFrameIndex(const std::vector<T>& keyFrames, float animationTime, int prevIndex, float prevAnimTime)
    {
        assert(prevIndex < int(keyFrames.size()));
        if(prevIndex < 0)
        {
            return GetFrameIndex(keyFrames, animationTime, 0, unsigned(keyFrames.size()));
        }
        assert(prevIndex >= 0);
        assert(prevAnimTime >= 0);
        if(animationTime >= prevAnimTime)
        {
            return GetFrameIndex(keyFrames, animationTime, prevIndex, unsigned(keyFrames.size()));
        }
        return GetFrameIndex(keyFrames, animationTime, 0, prevIndex);
    };

    static float GetScaleFactor(float start, float end, float animationTime)
    {
        //assert(animationTime >= start);
        //assert(end > start);
        const float progress = animationTime - start;
        const float total = end - start;
        //assert(progress <= total);
        return progress / total;
    }

    glm::mat4 interpolate_position(float animation_time)
    {
        assert(positions.size() > 0);
        if(positions.size() == 1)
        {
            return glm::translate(glm::mat4(1.0f), positions[0].position);
        }
        const int p0 = UpdateFrameIndex(positions, animation_time, prevPositionIndex, prevAnimationTime);
        prevPositionIndex = p0;
        const KeyPosition& prev = positions[p0];
        const KeyPosition& next = positions[p0 + 1];
        const float scale_factor = GetScaleFactor(prev.timeStamp, next.timeStamp, animation_time);
        const glm::vec3 position = glm::mix(prev.position, next.position, scale_factor);
        return glm::translate(glm::mat4(1.0f), position);
    }

    glm::mat4 interpolate_rotation(float animation_time)
    {
        assert(rotations.size() > 0);
        if(rotations.size() == 1)
        {
            return glm::mat4_cast(rotations[0].orientation);
        }
        const int p0 = UpdateFrameIndex(rotations, animation_time, prevRotationIndex, prevAnimationTime);
        prevRotationIndex = p0;
        const KeyRotation& prev = rotations[p0];
        const KeyRotation& next = rotations[p0 + 1];
        const float scale_factor = GetScaleFactor(prev.timeStamp, next.timeStamp, animation_time);
        const glm::quat orientation = glm::normalize(glm::slerp(prev.orientation, next.orientation, scale_factor));
        return glm::mat4_cast(orientation);
    }

    glm::mat4 interpolat_scaling(float animation_time)
    {
        assert(scales.size() > 0);
        if(scales.size() == 1)
        {
            return glm::scale(glm::mat4(1.0f), scales[0].scale);
        }
        const int p0 = UpdateFrameIndex(scales, animation_time, prevScaleIndex, prevAnimationTime);
        prevScaleIndex = p0;
        const KeyScale& prev = scales[p0];
        const KeyScale& next = scales[p0 + 1];
        const float scale_factor = GetScaleFactor(prev.timeStamp, next.timeStamp, animation_time);
        const glm::vec3 scale = glm::mix(prev.scale, next.scale, scale_factor);
        return glm::scale(glm::mat4(1.0f), scale);
    }
};

struct animNode
{
    std::optional<BoneKeyFrames> boneKeyFrames;
    glm::mat4 transform;
    
    int parent = -1;
    glm::mat4 localTransform;
    std::string debugName;
    int debugvertices = -1;
};
#pragma endregion

class Animation 
{
public:
    static constexpr std::size_t MaxBones = 100;
    Animation() = default;
    Animation(glm::mat4 rootInverse, std::vector<animNode>&& nodes, unsigned bones_count, float duration, float ticksPerSecond);
    static Animation CreateIdentityAnimation(unsigned int numBones)
    {
        std::vector<animNode> nodes;
        // Create a single root node
        animNode rootNode;
        rootNode.parent = -1;
        rootNode.localTransform = glm::mat4(1.0f); // Identity matrix
        rootNode.debugName = "static_root";
        nodes.push_back(std::move(rootNode));
    
        return Animation(glm::mat4(1.0f), std::move(nodes), numBones, 0.0f, 0.0f);
    }

    bool update(float dt);
    std::span<const glm::mat4> transforms() const;
    void debugDump() const;
private:
    glm::mat4 globalRootInverse;
    std::vector<glm::mat4> matrixTransforms;
    std::vector<animNode> nodes;
    unsigned bonesCount = 0;
    float currentTime = 0;
    float duration = 0;
    float ticksPerSecond = 0;
};