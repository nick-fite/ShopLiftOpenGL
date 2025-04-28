#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
//#include "src/Camera/Camera.h"
#include "src/AnimatedMeshes/Render/Render.h"
#include "src/AnimatedMeshes/Camera/Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



struct AppState {
    Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));
    float dt = 0.f;
    float lastFrameTime = 0.f;
    int ScreenWidth = 800;
    int ScreenHeight = 600;

};

static void OnWindowResize(GLFWwindow* window, int width, int height) {
    AppState* appState = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    appState->ScreenWidth = width;
    appState->ScreenHeight = height;

    glViewport(0, 0, width, height);
}

static void OnMouseMove(GLFWwindow* window, double xpos, double ypos) {
    AppState* appState = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    appState->camera.on_mouse_move(float(xpos), float(ypos));
}

static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
    AppState* appState = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    appState->camera.on_mouse_scroll(float(yoffset));
}

static void HandleInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    assert(app);
    Camera& camera = app->camera;

    const struct
    {
        int key;
        glm::vec3 delta;
    } key_delta[] =
    {
        {GLFW_KEY_W, +camera._front},
        {GLFW_KEY_S, -camera._front},
        {GLFW_KEY_D, +camera._right},
        {GLFW_KEY_A, -camera._right},
        {GLFW_KEY_E, +camera._up},
        {GLFW_KEY_Q, -camera._up},
    };

    for (const auto& [key, delta] : key_delta)
    {
        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            camera.on_keyboard_move(delta, app->dt);
            camera.force_refresh();
            break;
        }
    }
}

static void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    if ((key == GLFW_KEY_SPACE) && (action == GLFW_PRESS))
    {
        AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        assert(app);
        //old code for pausing, no want
        //app->_pause_animation ^= true;
    }
}

int main() {
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    AppState app;
    
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    
    GLFWwindow* window = glfwCreateWindow(mode->width/1.5, mode->height/1.5, "Model", NULL, NULL);
    glfwSetWindowUserPointer(window, &app);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, OnWindowResize);
    glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetScrollCallback(window, OnMouseScroll);
    glfwSetKeyCallback(window, OnKeyEvent);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    const char* const path = "D:/Profile Redirect/nfite/Desktop/NEW/ShopLiftOpenGL/assets/TestAssets/DanceTest/Dance.dae";
    const float model_scale = 0.012f;
    const int animation_index = -1;
    const float time_speed = 1.f;
    
    auto [renderModel, animation] = AssimpModel::LoadAnimatedModel(path, -1);

    app.camera.force_refresh();
    while(!glfwWindowShouldClose(window))
    {
        //std::fprintf(stderr, "FPS: %f\n", 1.f / app.dt);
        const float currentTime = float(glfwGetTime());
        app.dt = currentTime - app.lastFrameTime;
        app.lastFrameTime = currentTime;
        HandleInput(window);

        animation.update(app.dt * time_speed);

        if(app.ScreenHeight <= 0)
        {
            continue;
        }

        const glm::mat4 view = app.camera.view_matrix();
        const glm::mat4 projection = glm::perspective(glm::radians(app.camera._zoom), ((app.ScreenWidth * 1.f) / app.ScreenHeight), 0.1f, 10000.f);
        glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(model_scale));

        glClearColor(0.5f, 0.5f, 0.5f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderModel.draw(animation.transforms(), projection, view, model, glm::vec3(0.0f, 1.0f, 3.0f), app.camera._position);
        //std::fprintf(stderr, "Camera position: %f %f %f\n", view, app.camera._position.y, app.camera._position.z);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 1;
}