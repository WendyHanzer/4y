#include<GL/glew.h>
#include<GL/gl.h>
#include<glm/glm.hpp>
#include<string>
#include<utility>
#include<vector>
#define PI 3.14159265355


struct Maze_Holes 
{
    glm::mat4 model;
    glm::vec4 offset;
    std::pair<glm::vec4, glm::vec4> current_position;
    std::pair<glm::vec4, glm::vec4> initial_bound;
    bool last;
    bool first;

    GLint type;
    void init() 
    {
        model = glm::mat4(1.0f);
        offset = glm::vec4(0.0f);
        last = false;
        first = false;
    }
};


struct Mesh 
{
    glm::mat4 model;
    glm::vec4 offset;
    std::string meshName; 
    std::pair<glm::vec4, glm::vec4> current_position;    //current position - bounding box - pair.first = min, pair.second = max
    std::pair<glm::vec4, glm::vec4> initial_bound;       //initial position - bounding box - pair.first = min, pair.second = max
    glm::vec4 sensitivity;                               //sensitivity of the xz forces
    float bounciness;
    bool holeTopFlag;
    Maze_Holes whichHole;
    
    //speed in the axis
    glm::vec4 velocity; //speed
    
    //current acceleration in the axis
    glm::vec4 gravity;
    glm::vec4 xz_acceleration; //acceleration in xz vectors

    void initialize(std::string name) 
    {
        meshName = name;
        model = glm::mat4(1.0f); //identity matrix
        holeTopFlag = true;
        whichHole.init();

        offset = glm::vec4(0.0f);
        velocity = glm::vec4(0.0f);
        gravity = glm::vec4(0.0f, -9.8f, 0.0f, 0.0f);
        sensitivity = glm::vec4(10.0f, 0.0f, 10.0f,0.0f);
        bounciness = 0.5f;
    }


    //check for holes
    void checkBound(std::vector<Maze_Holes> &holes) 
    {
        holeTopFlag = onHoles(holes);
    }

    //make it so that the ball's x and z locations is on the hole, not the center point
    bool onHoles(std::vector<Maze_Holes> &holes) 
    {
        for(auto i : holes) 
        {
            if(i.first == false)
            {
               if(current_position.second.x < i.current_position.second.x && current_position.first.x > i.current_position.first.x)
               {
                   if(current_position.second.z < i.current_position.second.z && current_position.first.z > i.current_position.first.z) 
                   {
                       whichHole = i;
                       return false;
                   }
               }
           }
        }
        return true;
    }

    void updateBounds(float dt) 
    {
        xz_acceleration = (glm::transpose(model) * gravity )*sensitivity;
        velocity += xz_acceleration*dt;

        current_position.first = initial_bound.first + offset;
        current_position.second = initial_bound.second + offset;
    }
};
