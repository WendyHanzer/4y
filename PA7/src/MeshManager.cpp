#include"MeshManager.h"
#include<fstream>
#include<utility>
#include<stdio.h>

using namespace std;

MeshManager::MeshManager() 
{
}


MeshManager::~MeshManager() 
{
}

//load a model from an obj file
bool MeshManager::loadModel(string fileName, string name) 
{
    vector<glm::vec3> vertices;
    vector<glm::vec3> faces;
    vector<glm::vec3> normals;
    vector<glm::vec2> texcoords;

    vector<vertex> geometry;
    GLuint vbo_geometry;

    //import models
    Assimp::Importer importer;
    const aiScene* model = importer.ReadFile(fileName.c_str(), aiProcess_Triangulate |      //make triangles
                                                               aiProcess_SortByPType |      //if multiple meshes in the file
                                                               aiProcess_GenUVCoords |      //generate texture coordinates
                                                               aiProcess_GenSmoothNormals); //normals
    if(!model)
    {     
        return false;
    }


    for(unsigned int i = 0; i < model->mNumMeshes; i ++) 
    { 
        const aiMesh* mesh = model->mMeshes[i];

        //get the verticies and faces
        initMesh(mesh, vertices, faces, normals, texcoords);
        
        //build geometry
        buildGeometry(vertices, normals, faces, texcoords, geometry); // or vertices

        //create and save VBO
        glGenBuffers(1, &vbo_geometry);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
        glBufferData(GL_ARRAY_BUFFER,(geometry.size()*sizeof(vertex)), &geometry.front(),GL_STATIC_DRAW);

        vbo_objects.insert(pair<string, GLuint>(name, vbo_geometry));
        numVertices.insert(pair<string, int>(name, geometry.size()));
        min_max.insert(pair<string, pair<glm::vec4, glm::vec4> >(name, findBounds(vertices)));
    }

  return true;
}


void MeshManager::initMesh( const aiMesh* mesh, 
                            vector<glm::vec3> &vertices, 
                            vector<glm::vec3> &faces, 
                            vector<glm::vec3> &normals,
                            vector<glm::vec2> &texcoords) 
{
    glm::vec3 vertex;
    glm::vec3 face;
    glm::vec2 texCoord;

    for(unsigned int i = 0; i < mesh->mNumVertices; i ++) 
    {
        aiVector3D temp_vertex = mesh->mVertices[i];
        vertex.x = temp_vertex.x;
        vertex.y = temp_vertex.y;
        vertex.z = temp_vertex.z;
        vertices.push_back(vertex);
    }

    if(mesh->HasNormals()) 
    {
        for(unsigned int i = 0; i < mesh->mNumVertices; i ++) 
        {
            aiVector3D temp_vertex = mesh->mNormals[i];
            vertex.x = temp_vertex.x;
            vertex.y = temp_vertex.y;
            vertex.z = temp_vertex.z;
            normals.push_back(vertex);
        }
    }

    if( mesh->HasTextureCoords(0) )
    {
        for(unsigned int i = 0; i < mesh->mNumVertices; i ++) 
        {
            aiVector3D temp_vertex = mesh->mTextureCoords[0][i];
            texCoord.x = temp_vertex.x;
            texCoord.y = temp_vertex.y;
            texcoords.push_back(texCoord);
        }
    }

   for(unsigned int i = 0; i < mesh->mNumFaces; i ++) 
   {
        aiFace Face = mesh->mFaces[i];
        face.x = Face.mIndices[0];
        face.y = Face.mIndices[1];
        face.z = Face.mIndices[2];
        faces.push_back(face);
   }
}


//build the geometry from the verticies and faces
void MeshManager::buildGeometry(vector<glm::vec3> &vertices, 
                                vector<glm::vec3> &normals,  
                                vector<glm::vec3>&faces,
                                vector<glm::vec2> &texCoords, 
                                vector<vertex> &geometry) 
{
    vertex temp;
    for(auto i : faces) //for each face
    {
        for(int j = 0; j < 3; j ++) //for each vertex per face -- 3 vertices per face 
        {          
            for(int k = 0; k < 3; k ++) //x, y, and z 
            { 
                temp.position[k] = vertices[i[j]] [k];
                temp.normal[k] = normals[i[j]] [k];
            }

            //texture coordinates
            if(!texCoords.empty())
            {
                temp.texCoords[0] = texCoords[i[j]] [0];
                temp.texCoords[1] = texCoords[i[j]] [1];
            }
            geometry.push_back(temp);
        }
    }
}


pair<glm::vec4, glm::vec4> MeshManager::findBounds(vector<glm::vec3> &vertices) {
   glm::vec4 max;
     max.x = vertices[0].x;
     max.y = vertices[0].y;
     max.z = vertices[0].z;
     max.w = 0.0;
   glm::vec4 min;
     min.x = vertices[0].x;
     min.y = vertices[0].y;
     min.z = vertices[0].z;
     min.w = 0.0;

  for(auto i : vertices) {
     if(max.x < i.x)
       max.x = i.x;
     if(max.y < i.y)
       max.y = i.y;
     if(max.z < i.z)
       max.z = i.z;

     if(min.x > i.x)
       min.x = i.x;
     if(min.y > i.y)
       min.y = i.y;
     if(min.z > i.z)
       min.z = i.z;
  }
  return pair<glm::vec4, glm::vec4>(min, max);
}

GLuint MeshManager::getHandle(string handleName) {
  return vbo_objects[handleName];
}

int MeshManager::getNumVertices(string handleName) {
   return numVertices[handleName];
}

pair<glm::vec4, glm::vec4> MeshManager::getBounds(string handleName) {
   return min_max[handleName];
}
