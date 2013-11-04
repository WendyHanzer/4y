#include<GL/glew.h>
#include<GL/gl.h>
#include<glm/glm.hpp>
#include<string>
#include<utility>

#define PI 3.14159265355

struct Mesh 
{
    glm::mat4 model;
    glm::vec4 offset;
    std::string meshName; 
    std::pair<glm::vec4, glm::vec4> currentPos; //current position - bounding box - pair.first = min, pair.second = max
    std::pair<glm::vec4, glm::vec4> initialPos; //initial position - bounding box - pair.first = min, pair.second = max

    void initializeMesh(std::string name) 
    {
        meshName = name;
        model = glm::mat4(1.0f);
        offset = glm::vec4(0.0f);
    }

    void updateBounds(float dt) 
    {
        currentPos.first = initialPos.first + offset; //min xyz
        currentPos.second = initialPos.second + offset; //max xyz
    }
};
