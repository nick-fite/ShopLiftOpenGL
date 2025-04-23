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

struct MeshData {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;

    MeshData(glm::vec3 pos, glm::vec2 tex, glm::vec3 norm)
        : position(pos), texCoords(tex), normal(norm) {}
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

    void LoadMeshBone(const aiMesh* mesh, int meshNum);
    void LoadSingleBone(const aiBone* bone, int meshNum);

public:
    void DrawMesh();

};