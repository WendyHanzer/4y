#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting


#include "texture.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include <vector>

#include <assimp/Importer.hpp> // importer to read obj file
#include <assimp/scene.h> //aiScene object
#include <assimp/postprocess.h> // post-processing variables for the importer
#include <assimp/color4.h> // aiColor4 object, used to handle colors from mesh objects

#define INVALID_OGL_VALUE 0xFFFFFFFF
#define INVALID_MATERIAL 0XFFFFFFFF
#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;

struct Vertex
{
    GLfloat position[3];
    GLfloat color[3];
    GLfloat normal[3];
};

struct MeshEntry
{
    GLuint VB;
    GLuint IB;
    unsigned int NumIndices;
    unsigned int MaterialIndex;
    
    MeshEntry();
    ~MeshEntry();
};

MeshEntry::MeshEntry()
{
    VB = INVALID_OGL_VALUE;
    IB = INVALID_OGL_VALUE;
    NumIndices = 0;
    MaterialIndex = INVALID_MATERIAL;
}

MeshEntry::~MeshEntry()
{
    if (VB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &VB);
    }

    if (IB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &IB);
    }
}


struct Mesh
{
    std::vector<MeshEntry> myEntries;
    std::vector<Texture*> myTextures;
    
    Mesh();
    ~Mesh();
    void Clear();
};

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    Clear();
}

void Mesh::Clear()
{
    for (unsigned int i=0 ; i<myTextures.size(); i++) 
    {
        SAFE_DELETE(myTextures[i]);
    }
}

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

//assimp model loader
const aiScene* scene = NULL;
Mesh myMesh;

// variables to store max x, y to resize correctly
int mx, my;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);

//--Resource management
bool initialize( char* objectFile );
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
{
    // load inputfile name
    char *objectFile;
    objectFile = new char[strlen(argv[1])];
    strcpy(objectFile, argv[1]);

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    
    // Initialize all of our resources(shaders, geometry)
    bool init = initialize( objectFile );
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

////

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    for (unsigned int i = 0 ; i < myMesh.myEntries.size() ; i++) 
    {
        glBindBuffer(GL_ARRAY_BUFFER, myMesh.myEntries[i].VB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.myEntries[i].IB);

        const unsigned int MaterialIndex = myMesh.myEntries[i].MaterialIndex;
/*
        if (MaterialIndex < myMesh.myTextures.size() && myMesh.myTextures[MaterialIndex]) {
            myMesh.myTextures[MaterialIndex]->Bind(GL_TEXTURE0);
        }
*/
        glDrawElements(GL_TRIANGLES, myMesh.myEntries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);



////

/*
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset
    std::cout << "E" << std::endl;
    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));
    std::cout << "F" << std::endl;
    glDrawArrays(GL_TRIANGLES, 0, 12);//mode, starting index, count
    std::cout << "G" << std::endl; 
    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
  */                        
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    // flags to store spinning angle and status
    //total time
    //static float angle = 0.0;
    //float dt = getDT();// if you have anything moving, use dt

    //angle += dt * myPI/2; //move through 90 degrees a second

    model = glm::translate( glm::mat4(1.0f), glm::vec3(1.0, 1.0, 1.0));
    
    float scale;
    if( mx > my )
    {
       scale = mx; 
    }
    else
    {
        scale = my;
    }
    
    if ( scale <= 1.2 )
    {
        scale = 0.5;
    }
    else
    {
        scale /= 5;
    }
    model = glm::scale( model, glm::vec3(1.0/scale, 1.0/scale, 1.0/scale));

    // Update the state of the scene
    glutPostRedisplay();//call the display callback
}


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if(key == 27)//ESC
    {
        exit(0);
    }
}


bool initialize( char* objectFile )
{

    // check input file
    std::ifstream fin;
    fin.open( objectFile );
    if (!fin.good())
    {
        std::cout << "Invalid input file!" << std::endl;
        return 0;
    }
    fin.clear();
    fin.close();

    // import file, store into global scene object
    Assimp::Importer importer;
    scene = importer.ReadFile( objectFile, aiProcess_Triangulate );
    
    // If import failed, report it
    if ( !scene )
    {
        std::cout << importer.GetErrorString() << std::endl;
        return 0;
    }
    
    // scene object loaded
    std::cout << "Import of scene " << objectFile << " succeeded." << std::endl;

    // Initialize Mesh object by reading in number of meshes, materials
    myMesh.myEntries.resize(scene->mNumMeshes);
    myMesh.myTextures.resize(scene->mNumMaterials);

    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

    mx=0;
    my=0;

    // initialize meshes in the scene one by one
    for (unsigned int i=0; i<myMesh.myEntries.size(); i++)
    {
        const aiMesh* myaiMesh = scene->mMeshes[i];
        myMesh.myEntries[i].MaterialIndex = myaiMesh->mMaterialIndex;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
  
        for (unsigned int j=0; j<myaiMesh->mNumVertices; j++)
        {
            const aiVector3D* pPos = &(myaiMesh->mVertices[j]);
            const aiVector3D* pNormal = &(myaiMesh->mNormals[j]);
            const aiVector3D* pTexCoord = myaiMesh->HasTextureCoords(0) ? & (myaiMesh->mTextureCoords[0][j]) : &Zero3D;
          
            Vertex v;
            v.position[0] = pPos->x;
            v.position[1]= pPos->y;

            if( v.position[0] > mx )
            {
                mx = v.position[0];
            }
            
            if( v.position[1] > my )
            {
                my = v.position[1];
            }
            
            v.position[2] = pPos->z;
      
            v.color[0] = pTexCoord->x;
            v.color[1] = pTexCoord->y;

            // only do it if there is normal coordinate to avoid segfault
            if ( myaiMesh->HasNormals() )
            {
                v.normal[0] = pNormal->x;                      
                v.normal[1] = pNormal->y;
                v.normal[2] = pNormal->z;            
            }
            Vertices.push_back(v);
        }

        for (unsigned int j=0; j<myaiMesh->mNumFaces; j++) 
        {
            const aiFace& Face = myaiMesh->mFaces[j];
            assert(Face.mNumIndices == 3);
            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }
        
        myMesh.myEntries[i].NumIndices = Indices.size();
        glGenBuffers(1, &myMesh.myEntries[i].VB);
        glBindBuffer(GL_ARRAY_BUFFER, myMesh.myEntries[i].VB);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
        
        glGenBuffers(1, &myMesh.myEntries[i].IB);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.myEntries[i].IB);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * myMesh.myEntries[i].NumIndices, &Indices[0], GL_STATIC_DRAW);
        
  
    }

////////////////////////////

    // After initializing mesh, initialize materials
    std::string Filename;
    Filename = objectFile;
    
    // Extract the directory part from the file name
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) 
    {
        Dir = ".";
    }
    else if (SlashIndex == 0) 
    {
        Dir = "/";
    }
    else 
    {
        Dir = Filename.substr(0, SlashIndex);
    }
    
    // initialize the materials
    for (unsigned int i=0; i<scene->mNumMaterials; i++)
    {
        const aiMaterial* myMaterial = scene->mMaterials[i];
        myMesh.myTextures[i] = NULL;
        
        if (myMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString Path;
            if (myMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                std::string FullPath = Dir + "/" + Path.data;
                myMesh.myTextures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!myMesh.myTextures[i]->Load()) 
                {
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete myMesh.myTextures[i];
                    myMesh.myTextures[i] = NULL;
                    return false;
                }
                else 
                {
                    printf("Loaded texture '%s'\n", FullPath.c_str());
                }
            }
        }
    }
    
   
///////////////////////////////////


    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);



    // Load Shader Sources
    fin.open("../src/vshader.txt");

    char *vbuffer, *fbuffer;

    int charcount;
    
    if( !fin.good() )
    {
        std::cerr << "Failed to load shader input file.  Check if the file is with the makefile and is named vshader.txt" << '\n';
    }
    
    // load vshader.txt into buffer char array
    if ( fin.is_open() )
    {
        fin.seekg(0, std::ios::end);
        charcount = fin.tellg();
        vbuffer = new char[ charcount + 1 ];
        fin.seekg(0, std::ios::beg);
        fin.read( vbuffer, charcount );
        vbuffer[ charcount ] = '\0';
        fin.close();
    }

    // point *vs to contents in buffer
    const char *vs = vbuffer;

    fin.open("../src/fshader.txt");
    
    if( !fin.good() )
    {
        std::cerr << "Failed to load shader input file.  Check if the file is with the makefile and is named fshader.txt" << '\n';
    }
    
    // load fshader.txt into buffer char array
    if ( fin.is_open() )
    {
        fin.seekg(0, std::ios::end);
        charcount = fin.tellg();
        fbuffer = new char[ charcount + 1 ];
        fin.seekg(0, std::ios::beg);
        fin.read( fbuffer, charcount );
        fin.close();
        fbuffer[ charcount ] = '\0';
    }
    
    // point *fs to contents in buffer
    const char *fs = fbuffer;

    //compile the shaders
    GLint shader_status;

    // Vertex shader first
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }
    
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}


