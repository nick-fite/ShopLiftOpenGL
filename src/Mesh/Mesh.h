#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#define MAX_BONE_INFLUENCE 4

struct MeshData {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];

    MeshData(glm::vec3 pos, glm::vec2 tex, glm::vec3 norm, const int* boneIDs, const float* weights)
    : position(pos), texCoords(tex), normal(norm), tangent(0.0f, 0.0f, 0.0f)
    {
        // Copy arrays element by element
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            m_BoneIDs[i] = boneIDs[i];
            m_Weights[i] = weights[i];
        }
    }    
    MeshData(){}
};

class Mesh {
public:
    Mesh(std::vector<MeshData> vertices, std::vector<unsigned short> indices);
    Mesh(const aiMesh* mesh);
    ~Mesh();
private:
void MakeMesh(std::vector<MeshData> vertices, std::vector<unsigned short> indices);
    std::vector<MeshData> m_vertices;
    std::vector<unsigned short> m_indices;

    GLuint m_vertextBuffer;
    GLuint m_indexBuffer;
    
    void CalculateTangentSpace(std::vector<MeshData>& vertices, std::vector<unsigned short>& indices);
public:
    std::vector<MeshData> GetMeshData() {return m_vertices;};
    void DrawMesh();

};