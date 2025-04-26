#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

Mesh::Mesh(std::vector<MeshData> vertices, std::vector<unsigned short> indices)
{
    MakeMesh(vertices, indices);
}


Mesh::Mesh(const aiMesh* mesh)
{
    std::cout << "meshName: " << mesh->mName.data << std::endl;
    int t;
    for(t = 0; t < mesh->mNumVertices; ++t)
    {
        MeshData v;
        memcpy(&v.position, &mesh->mVertices[t], sizeof(glm::vec3));
        memcpy(&v.normal, &mesh->mNormals[t], sizeof(glm::vec3));
        if (mesh->mTextureCoords[0]) {
            //std::cout<< "Texture coordinates found for vertex " << t << std::endl;
            memcpy(&v.texCoords, &mesh->mTextureCoords[0][t], sizeof(glm::vec2));
        } else {
            //std::cout << "No texture coordinates found for vertex " << t << std::endl;
            v.texCoords = glm::vec2(0.0f, 0.0f); // Default value
        }
        

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

void Mesh::MakeMesh(std::vector<MeshData> vertices, std::vector<unsigned short> indices)
{
    vertices = vertices;
    indices = indices;

    CalculateTangentSpace(vertices, indices);

    //vertex buffer
    glGenBuffers(1, &m_vertextBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertextBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MeshData), &m_vertices[0], GL_STATIC_DRAW);

    //indicies buffer
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW);
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
    SetupAttribute(3,3, GL_FLOAT, MeshData, tangent);

    //turn on attribute
    for(int i = 0; i < 4; i++)
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
    
void Mesh::CalculateTangentSpace(std::vector<MeshData>& vertices, std::vector<unsigned short>& indices)
{
    for (size_t i = 0; i < indices.size(); i += 3) {
        MeshData& v0 = vertices[indices[i]];
        MeshData& v1 = vertices[indices[i + 1]];
        MeshData& v2 = vertices[indices[i + 2]];

        // Edges of the triangle
        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        // Texture coordinate differences
        glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
        glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        // Calculate tangent
        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        // Assign to vertices
        v0.tangent = tangent;
        v1.tangent = tangent;
        v2.tangent = tangent;
    }
}