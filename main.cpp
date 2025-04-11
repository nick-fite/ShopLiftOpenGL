#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "src/Mesh/Mesh.h"
#include "src/Transform/Transform3D.h"
#include "src/Shaders/Shader.h"
#include "src/Shaders/ShadersProgram/ShadersProgram.h"
#include <vector>
#include <iostream>
    
using namespace std;

glm::vec2 ViewportDimensions = glm::vec2(800, 600);
glm::vec2 MousePos = glm::vec2();

void resizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    ViewportDimensions = glm::vec2(width, height);
}


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    MousePos = glm::vec2(xpos, ypos);
}

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(ViewportDimensions.x, ViewportDimensions.y, "Model", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    Transform3D transform;
    transform.SetPosition(glm::vec3(0,0,-2));

    glewInit();

    Mesh* mesh = new Mesh("assets/TestPlayer.fbx");

    Shader* vertShader = new Shader("assets/Shaders/Vertex.glsl", GL_VERTEX_SHADER);
    Shader* fragShader = new Shader("\assets/Shaders/Fragment.glsl", GL_FRAGMENT_SHADER);
    
    ShaderProgram* shaderProgram = new ShaderProgram();
    shaderProgram->AttachShader(vertShader);
    shaderProgram->AttachShader(fragShader);

    while(!glfwWindowShouldClose(window))
    {
        float dt = glfwGetTime();
        glfwSetTime(0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0,0.0,0.0, 0.0);

        mesh->DrawMesh();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 1;
}