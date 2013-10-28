#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

#include<GL/glew.h>
#include<GL/gl.h>
#include<glm/glm.hpp>
#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<btBulletDynamicsCommon.h>
#include<assimp/Importer.hpp>
#include<assimp/postprocess.h>
#include<assimp/scene.h>



using namespace std;

//struct for holding all vertex information
struct vertex 
{
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat textureCoords[2];
};


class MeshManager 
{
    public:
        MeshManager();  //constructor
        ~MeshManager(); //destructor

        bool loadModel(string fileName, string name);               //loads the model from the given file and stores the handle
        GLuint getHandle(string);                                   //returns the handle of a specific handle
        int getNumVertices(string handleName);                      //returns the number of vertices of a loaded object
        pair<glm::vec4, glm::vec4> getBounds(string handleName);    //returns the min and max xyz values of the object(bounding box)

    private:
        //private method to retrieve the vertices and faces from assimp
        void initMesh(  const aiMesh* mesh, 
                        vector<glm::vec3> &vertices, 
                        vector<glm::vec3> &faces, 
                        vector<glm::vec3> &normals,
                        vector<glm::vec2> &texcoords); 

        //build geometry from the vertices and faces
        void buildGeometry( vector<glm::vec3> &vertices,
                            vector<glm::vec3> &normals,
                            vector<glm::vec3> &faces,
                            vector<glm::vec2> &texCoords,
                            vector<vertex> &geometry); 

        pair<glm::vec4, glm::vec4> findBounds(vector<glm::vec3> &vertices); //find the bounding box of the loaded object
        map<string, GLuint> vbo_objects;                                    //map to keep track of vbo objects
        map<string, int> numVertices;                                       //to keep track of the number of vertices per object
        map<string, pair<glm::vec4, glm::vec4> > min_max;                   //keep track of the bounding box of an object
};
#endif
