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


struct Player 
{
    bool isIdle = true;
    bool isWalking = false;
    bool isRunning = false;
    bool isStealing = false;
};


struct AppState {
    Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));
    float dt = 0.f;
    float lastFrameTime = 0.f;
    int ScreenWidth = 800;
    int ScreenHeight = 600;
    glm::vec3 playerPos = glm::vec3(0.f);
    float playerRot = 0.f;

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

static glm::vec3 sunPos = glm::vec3(-5.f, 15.f, 55.f);

static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
    sunPos.z += float(yoffset);
    std::fprintf(stderr, "sunPos: %f\n", sunPos.z);
    //position.y += float(yoffset);
    //AppState* appState = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    //appState->camera.on_mouse_scroll(float(yoffset));
}

/*static void HandleInput(GLFWwindow* window)
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
}*/

static void HandleInput(GLFWwindow* window, float dt, glm::vec3& poosition, float& rot, Player& playerState)
{
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        playerState.isIdle = false;
        playerState.isWalking = false;
        playerState.isRunning = false;
        playerState.isStealing = true;
        return;
    }
    if(playerState.isStealing)
    {
        playerState.isIdle = false;
        return;
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        playerState.isWalking = false;
        playerState.isRunning = true;       
    }
    else
    {
        playerState.isRunning = false;
    }

    float speed = playerState.isRunning ? 9.f : 5.0f;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        rot = 0.0f;
        playerState.isIdle = false;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        poosition += glm::vec3(0.f, 0.f, speed * dt);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        rot = 180.0f;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        playerState.isWalking = true;
        poosition += glm::vec3(0.f, 0.f, -speed * dt);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        
        rot = 90.0f;
        playerState.isIdle = false;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        poosition += glm::vec3(speed * dt, 0.f, 0.f);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        rot = -90.0f;
        playerState.isIdle = false;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        poosition += glm::vec3(-speed * dt, 0.f, 0.f);
    }
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        std::fprintf(stderr, "position: %d, %d, %d\n", poosition.x, poosition.y, poosition.z);
    }
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
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
    Player playerState;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    
    GLFWwindow* window = glfwCreateWindow(mode->width/1.5, mode->height/1.5, "Model", NULL, NULL);
    glfwSetWindowUserPointer(window, &app);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, OnWindowResize);
    //glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetScrollCallback(window, OnMouseScroll);
    glfwSetKeyCallback(window, OnKeyEvent);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    const char* const worldPath = "D:/Profile Redirect/nfite/Desktop/NEW/ShopLiftOpenGL/assets/Map/Scene.fbx";
    const char* const playerIdle = "D:/Profile Redirect/nfite/Desktop/NEW/ShopLiftOpenGL/assets/player/testKidTheif_Idle_withEverything.dae";
    const char* const playerWalk = "D:/Profile Redirect/nfite/Desktop/NEW/ShopLiftOpenGL/assets/player/testKidTheif_Walk_withEverything.dae";
    const char* const playerRun = "D:/Profile Redirect/nfite/Desktop/NEW/ShopLiftOpenGL/assets/player/testKidTheif_Run_withEverything.dae";
    const char* const playerSteal = "D:/Profile Redirect/nfite/Desktop/NEW/ShopLiftOpenGL/assets/player/testKidTheif_Steal_withEverything.dae";


    const float model_scale = 0.012f;
    const int animation_index = -1;
    const float time_speed = 1.f;
    
    auto [worldModel, worldAnim] = AssimpModel::LoadAnimatedModel(worldPath, -1, true);
    auto [KidIdle, KidIdleAnim] = AssimpModel::LoadAnimatedModel(playerIdle, -1, false);
    auto [kidWalk, KidWalkAnim] = AssimpModel::LoadAnimatedModel(playerWalk, -1, false);
    auto [KidRun, KidRunAnim] = AssimpModel::LoadAnimatedModel(playerRun, -1, false);
    auto [KidSteal, KidStealAnim] = AssimpModel::LoadAnimatedModel(playerSteal, -1, false);
    
    app.camera._position = glm::vec3(-5.f, 18.5f, -11.9f);
    app.camera._yaw = -269.2f;
    app.camera._pitch = -51.2f;

    //glm::vec3 sunPos = glm::vec3(-3.f, 10.f, 0.f);

    app.camera.force_refresh();

    while(!glfwWindowShouldClose(window))
    {
        //std::fprintf(stderr, "FPS: %f\n", 1.f / app.dt);
        const float currentTime = float(glfwGetTime());
        app.dt = currentTime - app.lastFrameTime;
        app.lastFrameTime = currentTime;
        //HandleInput(window);

        //KidAnim.update(app.dt * time_speed);

        if(app.ScreenHeight <= 0)
        {
            continue;
        }


        glClearColor(0.5f, 0.5f, 0.5f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view = app.camera.view_matrix();
        const glm::mat4 projection = glm::perspective(glm::radians(app.camera._zoom), ((app.ScreenWidth * 1.f) / app.ScreenHeight), 0.1f, 10000.f);
        glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(model_scale));
        worldModel.draw(worldAnim.transforms(), projection, view, model, sunPos, app.camera._position);

        playerState.isWalking = false;
        playerState.isIdle = true;
        HandleInput(window, app.dt, app.playerPos, app.playerRot, playerState);

        glm::mat4 playerMatrix = glm::mat4(1.0f);
        playerMatrix = glm::translate(playerMatrix, app.playerPos);
        playerMatrix = glm::rotate(playerMatrix, glm::radians(app.playerRot), glm::vec3(0.0f, 1.0f, 0.0f));
        playerMatrix = glm::scale(playerMatrix, glm::vec3(model_scale * 1.5));
        
        std::fprintf(stderr, "playerPos: %f %f %f\n", app.playerPos.x, app.playerPos.y, app.playerPos.z);

        if(playerState.isWalking)
        {
            KidWalkAnim.update(app.dt * time_speed);
            kidWalk.draw(KidWalkAnim.transforms(), projection, view, playerMatrix, sunPos, app.camera._position);
        }
        if(playerState.isRunning)
        {
            KidRunAnim.update(app.dt * time_speed);
            KidRun.draw(KidRunAnim.transforms(), projection, view, playerMatrix, sunPos, app.camera._position);
        }
        if(playerState.isStealing)
        {
            bool isDone = KidStealAnim.update(app.dt * time_speed);
            KidSteal.draw(KidStealAnim.transforms(), projection, view, playerMatrix, sunPos, app.camera._position);
            if(isDone)
            {
                playerState.isStealing = false;
            }
        }
        if(playerState.isIdle)
        {
            KidIdleAnim.update(app.dt * time_speed);
            KidIdle.draw(KidIdleAnim.transforms(), projection, view, playerMatrix, sunPos, app.camera._position);
        }


        
        
        //KidModel.draw(KidAnim.transforms(), projection, view, playerMatrix, glm::vec3(0.0f, 1.0f, 3.0f), app.camera._position);
        //std::fprintf(stderr, "Camera position: %f %f %f\n", view, app.camera._position.y, app.camera._position.z);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 1;
}