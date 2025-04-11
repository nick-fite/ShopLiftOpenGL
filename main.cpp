#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "src/Mesh.h"
#include "src/Transform3D.h"
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

    GLFWwindow* window = glfwCreateWindow(ViewportDimensions.x, ViewportDimensions.y, "Hello World", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    Transform3D transform;
    transform.SetPosition(glm::vec3(0,0,-2));

    glewInit();

    Mesh* mesh = new Mesh("assets/TestPlayer.fbx");

    
    



    cout << "hello world!" << endl;
    return 1;
}