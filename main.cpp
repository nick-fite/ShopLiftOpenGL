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

struct Item{
    float xStart;
    float xEnd;
    float yStart;
    float yEnd;
    bool isLeft = false;
    int index;
};


struct AppState {
    Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));
    float dt = 0.f;
    float lastFrameTime = 0.f;
    int ScreenWidth = 800;
    int ScreenHeight = 600;
    glm::vec3 playerPos = glm::vec3(0.f);
    float playerRot = 0.f;
    glm::vec3 managerPos = glm::vec3(0.f);
    float managerRot = 0.f;

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

static glm::vec3 sunPos = glm::vec3(-5.f, 15.f, -12.f);

static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
    //sunPos.z += float(yoffset);
    //std::fprintf(stderr, "sunPos: %f\n", sunPos.z);
    //position.y += float(yoffset);
    //AppState* appState = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    //appState->camera.on_mouse_scroll(float(yoffset));
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

bool isInRestrictedZone(const glm::vec3& position) {
    // Define the restricted x-coordinate ranges
    const struct {
        float min;
        float max;
    } restrictedRanges[] = {
        {-13.78f, -10.35f},
        {-9.6f, -6.35f},
        {-5.09f, -2.1f},
        {-0.9f, 2.5f}
    };
    
    if(position.x > 3.5f || position.x < -14.5f)
    {
        return true;
    }

    // Check if position is within any of the ranges
    for (const auto& range : restrictedRanges) {
        if (position.x >= range.min && position.x <= range.max) {
            if(position.z > -0.9 || position.z < -5.f)
            {
                return false;
            }
            return true;
        }
    }
    
    return false;
}

// Item storage map with file path -> position mapping
std::map<const char*, Item> itemPositions;

// Function to add item to the map

bool isPlayerNearItem(const glm::vec3& playerPos, const Item& item) {
    // Check if player is within the bounds of the item
    bool withinX = (playerPos.x >= item.xStart && playerPos.x <= item.xEnd);
    bool withinZ = (playerPos.z >= item.yStart && playerPos.z <= item.yEnd);
    
    return withinX && withinZ;
}

std::vector<std::shared_ptr<RenderModel>> itemModels;
std::vector<std::shared_ptr<Animation>> itemAnims;
bool isLeft = false;
/*
static void HandleInput(GLFWwindow* window, float dt, glm::vec3& poosition, float& rot, Player& playerState)
{
    isLeft = false;
    
    

    if(playerState.isStealing) return;

    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        
        const char* itemToRemove = nullptr;
        int indexToRemove = -1;
        
        for (const auto& [itemPath, itemData] : itemPositions) {
            if (isPlayerNearItem(poosition, itemData)) {
                // Found an item to steal
                itemToRemove = itemPath;
                indexToRemove = itemData.index;
                break; // Only steal one item per press
            }
        }
        
        if (itemToRemove != nullptr && indexToRemove >= 0 && indexToRemove < itemModels.size()) {
            // Start stealing animation
            playerState.isIdle = false;
            playerState.isWalking = false;
            playerState.isRunning = false;
            playerState.isStealing = true;
        
            // Debug output
            std::fprintf(stderr, "Stealing item at index %d\n", indexToRemove);
        
            // Remove the item from render vectors
            itemModels.erase(itemModels.begin() + indexToRemove);
            itemAnims.erase(itemAnims.begin() + indexToRemove);
        
            // Update indices of remaining items
            for (auto& [path, item] : itemPositions) {
                if (item.index > indexToRemove) {
                    item.index--; // Decrement indices of items after the removed one
                }
            }
        
            // Remove from map and free memory
            itemPositions.erase(itemToRemove);
            free((void*)itemToRemove);
        }

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
    bool moved = false;
    glm::vec3 originalPosition = poosition;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        rot = 0.0f;
        playerState.isIdle = false;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        poosition += glm::vec3(0.f, 0.f, speed * dt);
        moved = true;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        rot = 180.0f;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        playerState.isIdle = false;
        poosition += glm::vec3(0.f, 0.f, -speed * dt);
        moved = true;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        isLeft = true;
        rot = 90.0f;
        playerState.isIdle = false;
        if(!playerState.isRunning)
        {
            playerState.isWalking = true;
        }
        poosition += glm::vec3(speed * dt, 0.f, 0.f);
        moved = true;
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
        moved = true;
    }


    if(moved && isInRestrictedZone(poosition))
    {
        poosition = originalPosition;
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
*/
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

// Function to clean up allocated memory when done
void CleanupItems() {
    for (auto& [path, _] : itemPositions) {
        free((void*)path);
    }
    itemPositions.clear();
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
    glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetScrollCallback(window, OnMouseScroll);
    glfwSetKeyCallback(window, OnKeyEvent);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    
    
    const float model_scale = 0.1f;
    const int animation_index = -1;
    const float time_speed = 1.f;
    
    auto [KidIdle, KidIdleAnim] = AssimpModel::LoadAnimatedModel("/Users/nicholasfite/Desktop/ShopLiftOpenGL/assets/TestAssets/SpaceGuy.fbx", -1, false, 0x660000);

    int i = 0;
    for (const auto& [itemPath, itemData] : itemPositions) {
        itemPositions[itemPath].index = i;
        auto [itemModel, itemAnim] = AssimpModel::LoadAnimatedModel(itemPath, -1, true, 0x006600);
        itemModels.push_back(std::make_shared<RenderModel>(std::move(itemModel)));
        itemAnims.push_back(std::make_shared<Animation>(std::move(itemAnim)));
        i++;
    }


    app.camera._position = glm::vec3(-5.f, 18.5f, -11.9f);
    app.camera._yaw = -269.2f;
    app.camera._pitch = -51.2f;

    app.camera.force_refresh();

    while(!glfwWindowShouldClose(window))
    {
        //std::fprintf(stderr, "FPS: %f\n", 1.f / app.dt);
        const float currentTime = float(glfwGetTime());
        app.dt = currentTime - app.lastFrameTime;
        app.lastFrameTime = currentTime;




        if(app.ScreenHeight <= 0)
        {
            continue;
        }


        glClearColor(0.5f, 0.5f, 0.5f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view = app.camera.view_matrix();
        const glm::mat4 projection = glm::perspective(glm::radians(app.camera._zoom), ((app.ScreenWidth * 1.f) / app.ScreenHeight), 0.1f, 10000.f);
        glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(model_scale));

        playerState.isWalking = false;
        playerState.isIdle = true;
        HandleInput(window);

        glm::mat4 playerMatrix = glm::mat4(1.0f);
        playerMatrix = glm::translate(playerMatrix, app.playerPos);
        playerMatrix = glm::rotate(playerMatrix, glm::radians(app.playerRot), glm::vec3(0.0f, 1.0f, 0.0f));
        playerMatrix = glm::scale(playerMatrix, glm::vec3(model_scale * 1.5));
        
            KidIdleAnim.update(app.dt * time_speed);
            KidIdle.draw(KidIdleAnim.transforms(), projection, view, playerMatrix, sunPos, app.camera._position);
        
        //model = glm::translate(model, glm::vec3(50.f, 0.f, 4.f));
        //managerIdleAnim.update(app.dt * time_speed);
        //managerIdleModel.draw(managerIdleAnim.transforms(), projection, view, model, sunPos, app.camera._position);
        //managerWalkAnim.update(app.dt * time_speed);
        //ManagerWalkModel.draw(managerWalkAnim.transforms(), projection, view, model, sunPos, app.camera._position);


        //itemTes.draw(itemTestAnim.transforms(), projection, view, model, sunPos, app.camera._position);
        
        //KidModel.draw(KidAnim.transforms(), projection, view, playerMatrix, glm::vec3(0.0f, 1.0f, 3.0f), app.camera._position);

        //std::fprintf(stderr, "Camera position: %f %f %f\n", view, app.camera._position.y, app.camera._position.z);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 1;
}