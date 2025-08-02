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
const char* AddItem(const std::string& filePath, float xStart, float xEnd, float yStart, float yEnd, bool isLeft) {
    // Allocate memory for the path string that persists after function returns
    const char* pathCopy = _strdup(filePath.c_str());
    
    // Store in the map
    itemPositions[pathCopy].xStart = xStart;
    itemPositions[pathCopy].xEnd = xEnd;
    itemPositions[pathCopy].yStart = yStart;
    itemPositions[pathCopy].yEnd = yEnd;
    itemPositions[pathCopy].isLeft = isLeft;
    //itemPositions[pathCopy].index = index;
    
    // Return the pointer for reference
    return pathCopy;
}

bool isPlayerNearItem(const glm::vec3& playerPos, const Item& item) {
    // Check if player is within the bounds of the item
    bool withinX = (playerPos.x >= item.xStart && playerPos.x <= item.xEnd);
    bool withinZ = (playerPos.z >= item.yStart && playerPos.z <= item.yEnd);
    
    return withinX && withinZ;
}

std::vector<std::shared_ptr<RenderModel>> itemModels;
std::vector<std::shared_ptr<Animation>> itemAnims;
bool isLeft = false;

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
    //glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetScrollCallback(window, OnMouseScroll);
    glfwSetKeyCallback(window, OnKeyEvent);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char* const worldPath = "assets/Map/SceneNoItems.fbx";
    const char* const playerIdle = "assets/player/testKidTheif_Idle_withEverything.dae";
    const char* const playerWalk = "assets/player/testKidTheif_Walk_withEverything.dae";
    const char* const playerRun = "assets/player/testKidTheif_Run_withEverything.dae";
    const char* const playerSteal = "assets/player/testKidTheif_Steal_withEverything.dae";

    const char* const ManagerIdle = "assets/Manager/testManager_Idle_withEverything.dae";
    const char* const ManagerWalk = "assets/Manager/testManager_Run_withEverything.dae";

    const char* const item1 = "assets/Map/Item_GumBallsVar_1.fbx";
    const char* const item2 = "assets/Map/Item_GumBallsVar_2.fbx";
    const char* const item3 = "assets/Map/Item_GumBallsVar_3.fbx";
    const char* const item4 = "assets/Map/Item_GumBallsVar_4.fbx";
    const char* const item5 = "assets/Map/Item_GumBallsVar_5.fbx";
    const char* const item6 = "assets/Map/Item_GumBallsVar_6.fbx";
    const char* const item7 = "assets/Map/Item_GummyWormVar_1.fbx";
    const char* const item8 = "assets/Map/Item_GummyWormVar_2.fbx";
    //const char* const item9 = "assets/Map/Item_GummyWormVar_3.fbx";
    const char* const item10 = "assets/Map/Item_GummyWormVar_4.fbx";
    const char* const item11 = "assets/Map/Item_GummyWormVar_5.fbx";
    const char* const item12 = "assets/Map/Item_GummyWormVar_6.fbx";
    const char* const item13 = "assets/Map/Item_JawBreakerVar_1.fbx";
    const char* const item14 = "assets/Map/Item_JawBreakerVar_2.fbx";
    const char* const item15 = "assets/Map/Item_JawBreakerVar_3.fbx";
    const char* const item16 = "assets/Map/Item_JawBreakerVar_4.fbx";
    const char* const item17 = "assets/Map/Item_JawBreakerVar_5.fbx";
    const char* const item22 = "assets/Map/Item_MikeIke_.fbx";
    //const char* const item23 = "assets/Map/Item_MikeIke_.fbx";
    const char* const item24 = "assets/Map/Item_MikeIke_2.fbx";
    const char* const item25 = "assets/Map/Item_MikeIke_3.fbx";
    const char* const item26 = "assets/Map/Item_MikeIke_4.fbx";
    const char* const item27 = "assets/Map/Item_MikeIke_5.fbx";
    const char* const item28 = "assets/Map/Item_MikeIke_6.fbx";

    AddItem(item1, -14-1, -14+1, -0.8-1, -0.8+1, false);
    AddItem(item2, -13-1, -13+1, -3-1, -3+1, true);
    AddItem(item3, -9.6-1, -9.6+1, -1.8-1, -1.8+1, true);
    AddItem(item4, 2.8-1, 2.8+1, -2.8-1, -2.8+1, false);
    AddItem(item5, -10-1, -10+1, -4.2-1, -4.2+1, false);
    AddItem(item6, -1.8-1, -1.8+1, -2.8-1, -2.8+1, false);
    AddItem(item7, -14-1, -14+1, 0.8-1, 0.8+1, false);
    AddItem(item8, -14-1, -14+1, -1.7-1, -1.7+1, true);
    AddItem(item10, 2.8-1, 2.8+1, -1.6-1, -1.6+1, false);
    AddItem(item11, -5.18-1, -5.18+1, -2.9-1, -2.9+1, true);
    AddItem(item12, -1.8-1, -1.8+1, -4.2-1, -4.2+1, false);
    AddItem(item13, -14-1, -14+1, -2.39-1, -2.39+1, false);
    AddItem(item14, -13-1, -13+1, -4.73-1, -4.73+1, true);
    AddItem(item15, -5.18-1, -5.18+1, -4.3-1, -4.3+1, true);
    AddItem(item16, 2.8-1, 2.8+1, -4.3-1, -4.3+1, false);
    AddItem(item22, -10-1, -10+1, -2.8-1, -2.8+1, false);
    AddItem(item25, -10-1, -10+1, -1.8-1, -1.8+1, false);
    AddItem(item26, 3.84-1, 3.84+1, -2.7-1, -2.7+1, true);
    AddItem(item27, -5.18-1, -5.18+1, -2.8-1, -2.8+1, true);
    AddItem(item28, -6.17-1, -6.17+1, -1.46-1, -1.46+1, false);
    
    
    const float model_scale = 0.012f;
    const int animation_index = -1;
    const float time_speed = 1.f;
    
    auto [worldModel, worldAnim] = AssimpModel::LoadAnimatedModel(worldPath, -1, true);
    auto [KidIdle, KidIdleAnim] = AssimpModel::LoadAnimatedModel(playerIdle, -1, false, 0x660000);
    auto [kidWalk, KidWalkAnim] = AssimpModel::LoadAnimatedModel(playerWalk, -1, false, 0x660000);
    auto [KidRun, KidRunAnim] = AssimpModel::LoadAnimatedModel(playerRun, -1, false, 0x660000);
    auto [KidSteal, KidStealAnim] = AssimpModel::LoadAnimatedModel(playerSteal, -1, false, 0x660000);
    auto [managerIdleModel, managerIdleAnim] = AssimpModel::LoadAnimatedModel(ManagerIdle, -1, false, 0x660000);
    auto [ManagerWalkModel, managerWalkAnim] = AssimpModel::LoadAnimatedModel(ManagerWalk, -1, false, 0x660000);

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
        HandleInput(window);


        KidAnim.update(app.dt * time_speed);


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
        if(playerState.isIdle && !playerState.isStealing)
        {
            KidIdleAnim.update(app.dt * time_speed);
            KidIdle.draw(KidIdleAnim.transforms(), projection, view, playerMatrix, sunPos, app.camera._position);
        }


        
        for(size_t i = 0; i < itemModels.size(); ++i)
        {
            itemModels[i]->draw(itemAnims[i]->transforms(), projection, view, model, sunPos, app.camera._position);
            //itemMatrix = glm::rotate(itemMatrix, glm::radians(app.playerRot), glm::vec3(0.0f, 1.0f, 0.0f));
            //std::fprintf(stderr, "itemPos: %d %d %d\n", itemData.xStart, 0.f, itemData.yStart);
            //std::fprintf(stderr, "itemPos: %f %f %f\n", app.playerPos.x, app.playerPos.y, app.playerPos.z);
            //std::fprintf(stderr, "itemPos: %f %f %f\n", itemMatrix[3][0], itemMatrix[3][1], itemMatrix[3][2]);
            //std::fprintf(stderr, "itemPos: %d %d %d\n", itemData.xStart, 0.f, itemData.yStart);
            //std::fprintf(stderr, "itemPos: %f %f %f\n", app.playerPos.x, app.playerPos.y, app.playerPos.z);
            
            //itemModel.draw(itemAnim.transforms(), projection, view, model * itemMatrix, sunPos, app.camera._position);
        }
        
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