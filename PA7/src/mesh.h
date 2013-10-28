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
    std::pair<glm::vec4, glm::vec4> current_position;    //current position - bounding box - pair.first = min, pair.second = max
    std::pair<glm::vec4, glm::vec4> initial_bound;       //initial position - bounding box - pair.first = min, pair.second = max

    void initializeMesh(std::string name) 
    {
        meshName = name;
        model = glm::mat4(1.0f);
        offset = glm::vec4(0.0f);
    }

    void updateBounds() 
    {
        current_position.first = initial_bound.first + offset; //min xyz
        current_position.second = initial_bound.second + offset; //max xyz
    }
};
