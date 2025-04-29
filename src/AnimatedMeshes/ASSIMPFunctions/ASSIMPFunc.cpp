#include "ASSIMPFunc.h"
#include <stack>
AnimMesh AssimpAnimation::LoadMesh(const std::filesystem::path& path, const aiMesh& mesh, const aiScene& scene, BoneInfoRemap& boneInfoRemap)
{
        std::vector<AnimVertex> vertices;
    vertices.reserve(mesh.mNumVertices);
    for (unsigned i = 0; i < mesh.mNumVertices; ++i)
    {
        aiVector3D* uvs = mesh.mTextureCoords[0];
        assert(uvs);
        assert(mesh.mNormals);
        assert(mesh.mTangents);
        assert(mesh.mBitangents);
        vertices.push_back(AnimVertex{});
        AnimVertex& v = vertices.back();
        v.position = Vec_ToGLM(mesh.mVertices[i]);
        v.normal = Vec_ToGLM(mesh.mNormals[i]);
        v.texCoords = glm::vec2(uvs[i].x, uvs[i].y);
        v.tangent = Vec_ToGLM(mesh.mTangents[i]);
        v.bitangent = Vec_ToGLM(mesh.mBitangents[i]);
        std::fill(std::begin(v.m_BoneIDs), std::end(v.m_BoneIDs), -1);
        std::fill(std::begin(v.m_Weights), std::end(v.m_Weights), 0.f);
    }
    // Indices.
    std::vector<unsigned> indices;
    indices.reserve(mesh.mNumFaces * std::size_t(3));
    for (unsigned i = 0; i < mesh.mNumFaces; ++i)
    {
        aiFace face = mesh.mFaces[i];
        assert(face.mNumIndices == 3);
        for (unsigned j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
    // Textures.
    const aiMaterial* const material = scene.mMaterials[mesh.mMaterialIndex];
    assert(material);

    const struct TextureInfo
    {
        aiTextureType assimp_type;
        TextureType type;
    } kTexturesToFind[] =
    {
        {aiTextureType_DIFFUSE, TextureType::Diffuse},
        {aiTextureType_NORMALS, TextureType::Normal},
    };
    std::vector<AnimTexture> textures;
    for (auto [assimp_type, type] : kTexturesToFind)
    {
        // Get first (0) available texture of a given type.
        aiString file_name;
        if (material->GetTexture(assimp_type, 0, &file_name) == aiReturn_SUCCESS)
        {
            textures.push_back({});
            AnimTexture& t = textures.back();
            t.type = type;
            if (const aiTexture* texture = scene.GetEmbeddedTexture(file_name.C_Str()))
            {
                static_assert(sizeof(aiTexel) == 4);
                assert(texture->CheckFormat("png"));
                std::vector<std::uint8_t>& data = t.memoryTexture.png;
                data.resize(texture->mWidth);
                std::memcpy(data.data(), texture->pcData, texture->mWidth);
            }
            else
            {
                t.path = path.parent_path() / std::string(file_name.data, file_name.length);
                //assert(std::filesystem::exists(t.path));
            }
        }
    }

    // Bones weights for each vertex.
    auto add_bone_weight_to_vertex = [](AnimVertex& vertex, BoneIndex bone_index, float weight)
    {
        // Up to 4 influences supported. See AnimVertex passed to Vertex shader.
        auto it = std::find_if(std::begin(vertex.m_BoneIDs), std::end(vertex.m_BoneIDs)
            , [&](BoneIndex index) { return (index < 0) && (index != bone_index); });
        assert(it != std::end(vertex.m_BoneIDs)
            && "Either more then 4 bones per vertex OR duplicated bone.");
        const std::size_t i = std::distance(std::begin(vertex.m_BoneIDs), it);
        vertex.m_Weights[i] = weight;
        vertex.m_BoneIDs[i] = bone_index;
    };

    for (unsigned i = 0; i < mesh.mNumBones; ++i)
    {
        const aiString& bone_name = mesh.mBones[i]->mName;
        const BoneIndex bone_index = boneInfoRemap.AddNewBone(
            std::string(bone_name.data, bone_name.length)
            , Matrix_RowToColumn(mesh.mBones[i]->mOffsetMatrix));
        const aiBone* const bone = mesh.mBones[i];
        assert(bone);
        const aiVertexWeight* const weights = bone->mWeights;
        for (unsigned j = 0; j < bone->mNumWeights; ++j)
        {
            const unsigned vertex_id = weights[j].mVertexId;
            const float weight = weights[j].mWeight;
            assert(vertex_id <= vertices.size());
            add_bone_weight_to_vertex(vertices[vertex_id], bone_index, weight);
            boneInfoRemap.RecordVertex(bone_index);
        }
    }

    AnimMesh anim_mesh;
    anim_mesh.indices = std::move(indices);
    anim_mesh.vertices = std::move(vertices);
    anim_mesh.textures = std::move(textures);
    return anim_mesh;

}

std::vector<AnimMesh> AssimpAnimation::LoadModelMeshWithAnimationsWeights(const std::filesystem::path& modelPath, 
    const aiScene& scene, BoneInfoRemap& boneInfo)
{
    //std::fprintf(stderr, "OpenGL version: \n");
    std::vector<AnimMesh> meshes;

    std::stack<const aiNode*> dfs;
    dfs.push(scene.mRootNode);
    while (dfs.size() > 0)
    {
        const aiNode* const node = dfs.top();
        dfs.pop();

        for (unsigned i = 0; i < node->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene.mMeshes[node->mMeshes[i]];
            assert(mesh);
            meshes.push_back(LoadMesh(modelPath, *mesh, scene, boneInfo));
        }
        for (unsigned i = 0; i < node->mNumChildren; ++i)
        {
            dfs.push(node->mChildren[i]);
        }
    }
    return meshes;
}
BoneKeyFrames AssimpAnimation::LoadBoneKeyFrames(const aiNodeAnim& channel, const BoneMeshInfo& boneMeshInfo)
{
    BoneKeyFrames bone;
    bone.boneIndex = boneMeshInfo.index;
    bone.inverseBindPose = boneMeshInfo.inverseBindPose;

    assert(channel.mNumPositionKeys > 0);
    bone.positions.reserve(channel.mNumPositionKeys);
    for (unsigned index = 0; index < channel.mNumPositionKeys; ++index)
    {
        KeyPosition data;
        data.position = Vec_ToGLM(channel.mPositionKeys[index].mValue);
        data.timeStamp = float(channel.mPositionKeys[index].mTime);
        bone.positions.push_back(data);
    }

    assert(channel.mNumRotationKeys > 0);
    bone.rotations.reserve(channel.mNumRotationKeys);
    for (unsigned index = 0; index < channel.mNumRotationKeys; ++index)
    {
        KeyRotation data;
        data.orientation = Quat_ToGLM(channel.mRotationKeys[index].mValue);
        data.timeStamp = float(channel.mRotationKeys[index].mTime);
        bone.rotations.push_back(data);
    }

    assert(channel.mNumScalingKeys > 0);
    bone.scales.reserve(channel.mNumScalingKeys);
    for (unsigned index = 0; index < channel.mNumScalingKeys; ++index)
    {
        KeyScale data;
        data.scale = Vec_ToGLM(channel.mScalingKeys[index].mValue);
        data.timeStamp = float(channel.mScalingKeys[index].mTime);
        bone.scales.push_back(data);
    }
    
    return bone;

}

Animation AssimpAnimation::LoadAnimation(const aiScene& scene, int animationIndex, const BoneInfoRemap& boneInfoRemap)
{
if (scene.mNumAnimations == 0)
    {
        std::fprintf(stderr, "Assimp scene does not have animations. Loading invalid one (no-op).\n");
        return Animation();
    }
    if ((scene.mNumAnimations > 1) && (animationIndex < 0))
    {
        std::fprintf(stderr, "There are %u animations available."
            " Use '--animation N' to choose different animation. Loading first one.\n"
            , scene.mNumAnimations);
    }
    animationIndex = std::max(0, animationIndex); // Try to load first one, if nothing specified.
    //assert(unsigned(animationIndex) < scene.mNumAnimations);
    const aiAnimation* const animation = scene.mAnimations[animationIndex];
    const float duration = float(animation->mDuration);
    const float ticks_per_second = float(animation->mTicksPerSecond);
    std::vector<animNode> nodes;

    struct Node
    {
        const aiNode* src = nullptr;
        int parent = -1;
    };

    std::stack<Node> dfs;
    dfs.push(Node{scene.mRootNode, -1/*no parent*/});
    while (dfs.size() > 0)
    {
        Node data = std::move(dfs.top());
        dfs.pop();

        animNode node;
        node.parent = data.parent;
        node.localTransform = Matrix_RowToColumn(data.src->mTransformation);
        node.debugName = data.src->mName.C_Str();
        assert(node.parent < int(nodes.size()));
        nodes.push_back(std::move(node));
        const int parent_index = int(nodes.size() - 1);

        for (unsigned i = 0; i < data.src->mNumChildren; ++i)
        {
            dfs.push(Node{data.src->mChildren[i], parent_index});
        }
    }

    for (unsigned i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim* channel = animation->mChannels[i];
        const aiString& bone_name = channel->mNodeName;
        auto it = std::find_if(nodes.cbegin(), nodes.cend()
            , [&bone_name](const animNode& n)
        {
            return n.debugName == bone_name.C_Str();
        });
        assert(it != nodes.end() && "No node matching a bone.");
        const int index = int(std::distance(nodes.cbegin(), it));
        const BoneMeshInfo* info = boneInfoRemap.FindBone(bone_name.C_Str());
        if (!info)
        {
            std::fprintf(stderr, "No bone info for a node '%s' found.\n", bone_name.C_Str());
            continue;
        }
        animNode& node = nodes[index];
        assert(!node.boneKeyFrames.has_value() && "Two or more bones matching same node.");
        node.boneKeyFrames.emplace(LoadBoneKeyFrames(*channel, *info));
        node.debugvertices = boneInfoRemap.verticesCount(node.boneKeyFrames->boneIndex);
    }

    // Nodes with keyframes are all in from `animation->mNumChannels` above.
    // Still, setup bones with no keyframes so they participate as others
    // bones parent with `model_space_to_bone` transform.
    for (animNode& node : nodes)
    {
        if (node.boneKeyFrames)
        {
            continue;
        }
        const BoneMeshInfo* info = boneInfoRemap.FindBone(node.debugName.c_str());
        if (!info)
        {
            continue;
        }
        BoneKeyFrames& bone = node.boneKeyFrames.emplace();
        bone.boneIndex = info->index;
        bone.inverseBindPose = info->inverseBindPose;
        node.debugvertices = boneInfoRemap.verticesCount(node.boneKeyFrames->boneIndex);
        assert(!bone.hasAnyKeyframes());
    }

    const glm::mat4 root = Matrix_RowToColumn(scene.mRootNode->mTransformation);
    const unsigned bones_count = unsigned(boneInfoRemap.nameToInfo.size());
    return Animation(glm::inverse(root), std::move(nodes), bones_count, duration, ticks_per_second);
}
