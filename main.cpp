#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "src/Object/Object.h"
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

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    GLFWwindow* window = glfwCreateWindow(mode->width/1.5, mode->height/1.5, "Model", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    
    glewInit();
    Object* obj = new Object("../../assets/TestAssets/testPlayer.fbx", "JawBreaker");
    //Object* obj1 = new Object("../../assets/TestAssets/CandyStore_scene.fbx", "JawBreaker");
    
    Transform3D transform;
    transform.SetPosition(glm::vec3(0,0,-2));

    FPSController controller = FPSController();

    Shader* vertShader = new Shader("../../assets/Shaders/Vertex.glsl", GL_VERTEX_SHADER);
    Shader* fragShader = new Shader("../../assets/Shaders/Fragment.glsl", GL_FRAGMENT_SHADER);
    
    char cameraViewVS[] = "cameraView";
    char worldMatrixVS[] = "worldMatrix";
    char textureFS[] = "tex";

    char textureFile[] = "../../assets/TestAssets/Textures/Solid_gray.png";

    //ShaderProgram* shaderProgram = new ShaderProgram();
    //shaderProgram->AttachShader(vertShader);
    //shaderProgram->AttachShader(fragShader);

    //Material* mat = new Material(shaderProgram);
    //mat->SetTexture(textureFS, new Texture(textureFile));
    
    while(!glfwWindowShouldClose(window))
    {
        float dt = glfwGetTime();
        glfwSetTime(0);

        controller.Update(window, ViewportDimensions, MousePos, dt);

        glm::mat4 view = controller.GetTransform().GetInverseMatrix();
        glm::mat4 projection = glm::perspective(1.f, ViewportDimensions.x/ViewportDimensions.y, .1f, 1000.f);
        glm::mat4 viewProjection = projection * view;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0,0.0,0.0, 0.0);

        //mat->SetMatrix(cameraViewVS, viewProjection);
        //mat->SetMatrix(worldMatrixVS, transform.GetMatrix());

        //mat->Bind();

        obj->DrawMeshes();
        //obj1->DrawMeshes();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    //delete mat;
    //delete shaderProgram;
    delete vertShader;
    delete fragShader;
    delete obj;
    glfwTerminate();

    return 1;
}