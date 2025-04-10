#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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
    Mesh(std::vector<MeshData> vertices, std::vector<unsigned int> indices);
    Mesh(std::string filePath);
    ~Mesh();
private:
void MakeMesh(std::vector<MeshData> vertices, std::vector<unsigned int> indices);
    std::vector<MeshData> m_vertices;
    std::vector<unsigned int> m_indices;

    GLuint m_vertextBuffer;
    GLuint m_indexBuffer;
public:
    void DrawMesh();

};