#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
//#include "src/Camera/Camera.h"
#include "src/Render/Render.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Camera
{
    glm::vec3 _position = glm::vec3(0.f);
    glm::vec3 _front = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 _up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 _right = glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 _world_up = glm::vec3(0.f, 1.f, 0.f);
    float _yaw = -90.f;
    float _pitch = 0.f;
    float _movement_speed = 2.5f;
    float _mouse_sensitivity = 0.1f;
    float _zoom = 45.f;

#if !defined(NAN)
#  error Compiler does not support float's NAN.
#endif
    float _mouse_last_x = NAN;
    float _mouse_last_y = NAN;

    glm::mat4 view_matrix() const
    {
        return glm::lookAt(_position, _position + _front, _up);
    }

    void on_keyboard_move(glm::vec3 delta, float deltaTime)
    {
        float velocity = _movement_speed * deltaTime;
        _position += delta * velocity;
    }

    void on_mouse_scroll(float yoffset)
    {
        _zoom -= yoffset;
    }

    void on_mouse_move(float x, float y)
    {
        if (std::isnan(_mouse_last_x) || std::isnan(_mouse_last_y))
        {
            _mouse_last_x = x;
            _mouse_last_y = y;
        }
        const float xoffset = (x - _mouse_last_x) * _mouse_sensitivity;
        const float yoffset = (_mouse_last_y - y) * _mouse_sensitivity;
        _mouse_last_x = x;
        _mouse_last_y = y;
        _yaw += xoffset;
        _pitch += yoffset;
        force_refresh();
    }

    void force_refresh()
    {
        _pitch = std::clamp(_pitch, -89.0f, 89.0f);

        _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front.y = sin(glm::radians(_pitch));
        _front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front = glm::normalize(_front);
        _right = glm::normalize(glm::cross(_front, _world_up));
        _up = glm::normalize(glm::cross(_right, _front));
    }
};

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
            std::fprintf(stderr, "key: %d\n", key);
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

    
    auto [renderModel, animation] = AssimpModel::LoadAnimatedModel("../../assets/TestAssets/DanceTest/Dance.dae", 0);

    app.camera.force_refresh();
    while(!glfwWindowShouldClose(window))
    {
        //std::fprintf(stderr, "FPS: %f\n", 1.f / app.dt);
        const float currentTime = float(glfwGetTime());
        app.dt = currentTime - app.lastFrameTime;
        app.lastFrameTime = currentTime;
        HandleInput(window);

        animation.update(app.dt * 1.f);

        if(app.ScreenHeight <= 0)
        {
            continue;
        }

        app.camera._position = glm::vec3(0.f, 5.f, 10.f);

        const glm::mat4 view = app.camera.view_matrix();
        const glm::mat4 projection = glm::perspective(glm::radians(app.camera._zoom), ((app.ScreenWidth * 1.f) / app.ScreenHeight), 0.1f, 10000.f);
        glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));

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