#include "../mesh/Mesh.h"
#include "../Material/Material.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>

class Object {
private:
    //std::vector<Mesh*> meshes;
    std::map<std::string, Mesh*> meshMap;
public:
    Object(std::string filePath, std::string name);
    ~Object();
    std::string name;

    void DrawMeshes();
};