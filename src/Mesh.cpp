#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

Mesh::Mesh(std::vector<MeshData> vertices, std::vector<unsigned int> indices)
{
    MakeMesh(vertices, indices);
}


Mesh::Mesh(std::string filePath)
{
    std::ifstream file(filePath);
    
    if(!file.good())
    {
        std::cout << "bad file: " << filePath << std::endl;
        return;
    }

    Assimp::Importer importer;
    const aiScene* scene = NULL;
    unsigned int n = 0, t;

    for(int k = 0; k < 1; k++)
    {
        scene = importer.ReadFile(filePath, aiProcessPreset_TargetRealtime_Quality | aiProcess_PreTransformVertices);
    }

    //this what indicates how many meshes are being imported. If I take in multiple in the same fbx will this process it?
    const struct aiMesh* mesh = scene->mMeshes[0];
    
    for(t = 0; t < mesh->mNumVertices; ++t)
    {
        MeshData v;
        memcpy(&v.position, &mesh->mVertices[t], sizeof(glm::vec3));
        memcpy(&v.normal, &mesh->mNormals[t], sizeof(glm::vec3));
        memcpy(&v.texCoords, &mesh->mTextureCoords[0][t], sizeof(glm::vec2));

        m_vertices.push_back(v);
    }

    for(t = 0; t < mesh->mNumFaces; ++t)
    {
        const struct aiFace* face = &mesh->mFaces[t];

        for(int i = 0; i < 3; i++)
        {
            m_indices.push_back(face->mIndices[i]);
        }

        if(face->mNumIndices == 4)
        {
            m_indices.push_back(face->mIndices[0]);
            m_indices.push_back(face->mIndices[2]);
            m_indices.push_back(face->mIndices[3]);
        }
    }

    printf("verticies: %d, indicies: %d\n", (int)m_vertices.size(), (int)m_indices.size());
    MakeMesh(m_vertices, m_indices);

}
void Mesh::MakeMesh(std::vector<MeshData> vertices, std::vector<unsigned int> indices)
{
    vertices = vertices;
    indices = indices;

    //vertex buffer
    glGenBuffers(1, &m_vertextBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertextBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MeshData), &m_vertices[0], GL_STATIC_DRAW);

    //indicies buffer
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

Mesh::~Mesh() 
{
    glDeleteBuffers(1, &m_vertextBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
}

#define SetupAttribute(index, size, type, structure, element)\
    glVertexAttribPointer(index, size, type, 0, sizeof(structure), (void*)offsetof(structure, element));

void Mesh::DrawMesh()
{
    //bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_vertextBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    //setup attribute
    SetupAttribute(0,3,GL_FLOAT, MeshData, position);
    SetupAttribute(1,2,GL_FLOAT, MeshData, texCoords);
    SetupAttribute(2,3,GL_FLOAT, MeshData, normal);

    //turn on attribute
    for(int i = 0; i < 3; i++)
    {
        glEnableVertexAttribArray(i);
    }

    //draw it
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_SHORT, (void*)0);

    //unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //disable attribs
    for(int i = 0; i < 3; i++)
    {
        glDisableVertexAttribArray(i);
    }

}