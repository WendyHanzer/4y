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


Vertex *loadOBJ(char *fileName, int &size )
{
    //initialize variables
    std::ifstream fin;
    std::string tempStr;
    std::vector<glm::vec3> v;
    std::vector<glm::vec3> vn;
    std::vector<glm::vec3> vt;
    std::vector<int> vF, vnF, vtF;
    glm::vec3 temp;
    glm::vec3 colorTemp;
    glm::vec3 tempVertex;
    char identifier[100], delim;
    int vIndex[3], vnIndex[3], vtIndex[3];
    bool vtFlag = false;

    //int vtIndex[3];


    //open file
    fin.clear();
    fin.open( fileName );

    //check for valid file
    if (!fin.good())
    {
        return 0;
    }

    //prime loop; parse information
    fin >> identifier;
    
    //place all values into appropriate containers
    while( fin.good() )
    {        
        if (strcmp(identifier, "v") == 0) //for vertex
        {
            fin >> temp.x >> temp.y >> temp.z;
            v.push_back(temp);    
        }

        else if ( strcmp (identifier, "vt") == 0) //for texture
        {
            vtFlag = true;
            fin >> colorTemp.x >> colorTemp.y;
            colorTemp.z = 0.0;
            vt.push_back(colorTemp);      
        }

        else if (strcmp(identifier, "vn") == 0) //for normal
        {
            fin >> temp.x >> temp.y >> temp.z;
            vn.push_back(temp);
        }

        else if (strcmp(identifier, "f") == 0)  //for face
        {
            fin >> vIndex[0];

            //check to see what deliminating character is
            delim = fin.peek();

            if( vtFlag )
            {
                if(delim >= '0' && delim <= '9') //space delimited
                {
                    fin >> vtIndex[0] >> vnIndex[0]
                        >> vIndex[1] >> vtIndex[1] >> vnIndex[1]
                        >> vIndex[2] >> vtIndex[2] >> vnIndex[2];
                }
                else //character delimited (ie '/', ',', etc)
                {      
                    fin >> delim >> vtIndex[0] >> delim >> vnIndex[0]
                        >> vIndex[1] >> delim >> vtIndex[1] >> delim >> vnIndex[1]
                        >> vIndex[2] >> delim >> vtIndex[2] >> delim >> vnIndex[2];
                }
            }

            else
            {
                if(delim >= '0' && delim <= '9') //space delimited
                {
                    fin >> vIndex[0] >> vnIndex[0]
                        >> vIndex[1] >> vnIndex[1]
                        >> vIndex[2] >> vnIndex[2];
                }
                else //character delimited (ie '/', ',', etc)
                {      
                    fin >> delim >> delim >> vnIndex[0]
                        >> vIndex[1] >> delim >> delim >> vnIndex[1]
                        >> vIndex[2] >> delim >> delim >> vnIndex[2];
                }

            }

            vF.push_back(vIndex[0]);
            vF.push_back(vIndex[1]);
            vF.push_back(vIndex[2]);
            vnF.push_back(vnIndex[0]);
            vnF.push_back(vnIndex[1]);
            vnF.push_back(vnIndex[2]);            
            if (vtFlag )
            {
                vtF.push_back(vtIndex[0]);
                vtF.push_back(vtIndex[1]);
                vtF.push_back(vtIndex[2]);
            }
            else
            {
                vtF.push_back(0.0);
                vtF.push_back(0.0);
                vtF.push_back(0.0);
            }
                        
            
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
    int vFindex, vtFindex;
    size=0;
    for(unsigned int i = 0; i < vF.size(); i++)
    {
        vFindex = vF[i];
        vtFindex = vtF[i];
        
        //copy the vertex
        geometry[i].position[0] = v[vFindex-1].x;
        geometry[i].position[1] = v[vFindex-1].y;
        geometry[i].position[2] = v[vFindex-1].z; 

        //copy the texture
        geometry[i].color[0] = vt[vtFindex-1].x;
        geometry[i].color[1] = vt[vtFindex-1].y;
        geometry[i].color[2] = 0.0;
        size += 32*6;
        //copy the normal
            //none yet.

    }
    return geometry;
}
