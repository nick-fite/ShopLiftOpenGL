#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "src/Mesh/Mesh.h"
#include "src/Transform/Transform3D.h"
#include "src/Shaders/Shader.h"
#include "src/Shaders/ShadersProgram/ShadersProgram.h"
#include "src/Controllers/FPSController.h"
#include "src/Material/Material.h"
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

    
    glewInit();
    
    Mesh* mesh = new Mesh("D:\\profile redirect\\nfite\\Desktop\\ShopLiftOpenGL\\assets\\TestAssets\\TestPlayer.fbx");
    
    Transform3D transform;
    transform.SetPosition(glm::vec3(0,0,-2));

    FPSController controller = FPSController();    
    Shader* vertShader = new Shader("D:\\profile redirect\\nfite\\Desktop\\ShopLiftOpenGL\\assets\\Shaders\\Vertex.glsl", GL_VERTEX_SHADER);
    Shader* fragShader = new Shader("D:\\profile redirect\\nfite\\Desktop\\ShopLiftOpenGL\\assets\\Shaders\\Fragment.glsl", GL_FRAGMENT_SHADER);
    
    char cameraViewVS[] = "cameraView";
    char worldMatrixVS[] = "worldMatrix";
    char textureFS[] = "tex";

    char textureFile[] = "D:\\profile redirect\\nfite\\Desktop\\ShopLiftOpenGL\\assets\\TestAssets\\Textures\\Solid_blue.png";

    ShaderProgram* shaderProgram = new ShaderProgram();
    shaderProgram->AttachShader(vertShader);
    shaderProgram->AttachShader(fragShader);

    Material* mat = new Material(shaderProgram);
    mat->SetTexture(textureFS, new Texture(textureFile));
    

    std::cout << "hello world" << std::endl << std::endl;

    while(!glfwWindowShouldClose(window))
    {
        float dt = glfwGetTime();
        glfwSetTime(0);

        controller.Update(window, ViewportDimensions, MousePos, dt);

        glm::mat4 view = controller.GetTransform().GetInverseMatrix();
        glm::mat4 projection = glm::perspective(.75f, ViewportDimensions.x/ViewportDimensions.y, .1f, 100.f);
        glm::mat4 viewProjection = projection * view;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0,0.0,0.0, 0.0);

        mat->SetMatrix(cameraViewVS, view);
        mat->SetMatrix(worldMatrixVS, transform.GetMatrix());

        mat->Bind();
        

        mesh->DrawMesh();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 1;
}