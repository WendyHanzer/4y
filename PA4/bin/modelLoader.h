#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
};


Vertex *loadOBJ(char *fileName )
{
    //initialize variables
    std::ifstream fin;
    std::string tempStr;
    std::vector<glm::vec3> v;
    std::vector<glm::vec3> vn;
    std::vector<glm::vec2> vt;
    std::vector<int> vF, vnF, vtF;
    glm::vec3 temp;
    glm::vec2 colorTemp;
    glm::vec3 tempVertex;
    char identifier[100], trash;
    int vIndex[3], vnIndex[3];
    //int vtIndex[3];


    //open file
    fin.clear();
    fin.open( fileName );

    //check for valid file
    if (!fin.good())
    {
        //prime loop; parse information
        fin >> identifier;
    }

    
    //place all values into appropriate containers
    while( fin.good())
    {        
        if (strcmp(identifier, "v") == 0) //for vertex
        {
            fin >> temp.x >> temp.y >> temp.z;
            v.push_back(temp);    
        }

        else if ( strcmp (identifier, "vt") == 0) //for texture
        {
            fin >> temp.x >> temp.y >> temp.z;
            vt.push_back(colorTemp);      
        }

        else if (strcmp(identifier, "vn") == 0) //for normal
        {
            fin >> colorTemp.x >> colorTemp.y;
            vn.push_back(temp);
        }

        else if (strcmp(identifier, "f") == 0)  //for face
        {
            fin >> vIndex[0] >> trash >> /*vtIndex[0] >> */ trash >> vnIndex[0]
                >> vIndex[1] >> trash >> /*vtIndex[1] >> */ trash >> vnIndex[1]
                >> vIndex[2] >> trash >> /*vtIndex[2] >> */ trash >> vnIndex[2];

            vF.push_back(vIndex[0]);
            vF.push_back(vIndex[1]);
            vF.push_back(vIndex[2]);
            //vtF.push_back(vtIndex[0]);
            //vtF.push_back(vtIndex[0]);
            //vtF.push_back(vtIndex[0]);
            vnF.push_back(vnIndex[0]);
            vnF.push_back(vnIndex[1]);
            vnF.push_back(vnIndex[2]);
        }

        else
        {
            std::getline(fin, tempStr);
        }

        fin >> identifier;
    }
    fin.close();

    //copy vectors into the Vertex struct
    Vertex *geometry = new Vertex[vF.size()];
    int index;

    for(unsigned int i = 0; i < vF.size(); i++)
    {
        index = vF[i];
        
        //copy the vertex
        geometry[i].position[0] = v[index-1].x;
        geometry[i].position[1] = v[index-1].y;
        geometry[i].position[2] = v[index-1].z; 

        //std::cout << v[index-1].x << ", " << v[index-1].y << ", " << v[index-1].z << std::endl;
        //copy the texture
        geometry[i].color[0] = 0.0;
        geometry[i].color[1] = 1.0;
        geometry[i].color[2] = 1.0;

        //copy the normal
            //none yet.

    }
    
    return geometry;
}
