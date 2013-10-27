/*
  Struct : Mesh
   - this is simply used to keep track of the position of each object
     by using a bounding box
   - it also stores the Model Matrix for an object
   - so essentially, it tells keeps track of where the object is in the world and
     the Model matrix for it
*/

#include<GL/glew.h>
#include<GL/gl.h>
#include<glm/glm.hpp>
#include<string>
#include<utility>


using namespace std;

#define PI 3.14159265355

//data structures
struct Mesh {
    glm::mat4 model; //model matrix

    string meshType; //name
    pair<glm::vec4, glm::vec4> current_position; //current position - bounding box - pair.first = min, pair.second = max
    pair<glm::vec4, glm::vec4> initial_bound; //initial position - bounding box - pair.first = min, pair.second = max

    //offset from the initial position
       glm::vec4 offset; //used to calculate the current position

    void initialize(string name) { //set everything to zero
      meshType = name;
      model = glm::mat4(1.0f); //identity matrix
      offset = glm::vec4(0.0f);  //no offset
    }

   void updateBounds() {
        current_position.first = initial_bound.first + offset; //min xyz
        current_position.second = initial_bound.second + offset; //max xyz
    }
};
