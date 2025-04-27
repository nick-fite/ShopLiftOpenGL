#include "Animation.h"
Animation::Animation(const std::string& animationPath, Object* obj)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    aiAnimation* animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    std::cout << "loading animation: " << animationPath << std::endl;
    ReadMissingBones(animation, obj);
}

Bone* Animation::FindBone(const std::string& name) const 
{
    std::vector<Bone*>::const_iterator itr = std::find_if(m_Bones.begin(), m_Bones.end(), 
        [&](const Bone* bone)  // Changed from reference to pointer
        {
            return bone->GetBoneName() == name;  // Use -> instead of .
        }
    );
    if(itr == m_Bones.end())
    {
        return nullptr;
    }
    else return (*itr);
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);

    dest.name = src->mName.C_Str();
    dest.transform = ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for(int i = 0; i < src->mNumChildren; ++i)
    {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

void Animation::ReadMissingBones(const aiAnimation* animation, Object* obj)
{
    int size = animation->mNumChannels;

		auto& boneInfoMap = obj->GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = obj->getBoneCount(); //getting the m_BoneCounter from Model class
// Print out the boneInfoMap before processing
    std::cout << "--- BoneInfoMap Contents Before Processing ---" << std::endl;
    std::cout << "size: " << size << std::endl;
    std::cout << "bonecount: " << boneCount << std::endl;
    if(boneInfoMap.empty()) {
        std::cout << "BoneInfoMap is empty!" << std::endl;
    } else {
        for(const auto& pair : boneInfoMap) {
            std::cout << "Bone Name: " << pair.first;
            if(pair.second) {
                std::cout << ", ID: " << pair.second->ID 
                          << ", Offset Matrix: " << glm::to_string(pair.second->offsetMatrix);
            } else {
                std::cout << " -> NULL POINTER";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "----------------------------------------" << std::endl;
    


		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
            auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;
            
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
                boneInfoMap[boneName]->ID = boneCount;
				boneCount++;
			}
            Bone* temp = new Bone(channel->mNodeName.data,boneInfoMap[channel->mNodeName.data]->ID, channel);
			m_Bones.push_back(temp);
		}

		m_BoneMap = boneInfoMap;
}
