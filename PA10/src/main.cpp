#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>

#include <glm/glm.hpp> //matrix operations
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <math.h>
#include <ImageMagick/Magick++.h> //image loader for textures

#include"MeshManager.h" //for loading models
#include "mesh.h"
#include"ShaderManager.h" //for managing shaders
//#include"Physics.h" //physics

//CONSTS used for user interactions
int w = 1024, h = 768;// Window size

struct Light
{
    GLfloat specularVec[3];
    GLfloat ambientVec[3];
    GLfloat diffuseVec[3];
    GLfloat lightVec[3];
    
    void Init();

};

void Light::Init()
{
    specularVec[0] = 1.0;
    specularVec[1] = 1.0;
    specularVec[2] = 1.0;
    
    ambientVec[0] = 0.2;
    ambientVec[1] = 0.2;
    ambientVec[2] = 0.2;
    
    diffuseVec[0] = 0.7;
    diffuseVec[1] = 0.7;
    diffuseVec[2] = 0.7;
    
    lightVec[0] = 10.0;
    lightVec[1] = 10.0;
    lightVec[2] = 20.0;
}





bool specularOn = true;
bool ambientOn = true;
bool diffuseOn = true;


MeshManager meshManager;
ShaderManager *shaderManager;
GLuint tanTex, green, blue;
GLint textureCoord;
GLint TexUnit;

GLint scene_mv;
GLint scene_p;
GLint scene_position;
GLint scene_normal;

GLint light_position;
GLint diffuse_data;
GLint specular_data;
GLint ambient_data;

Light Lighting;

//meshes
Mesh sphere;
Mesh tempMesh;

//transform matrices
glm::mat4 view;         //world->eye
glm::mat4 projection;   //eye->clip
glm::mat4 mv;           //model-view matrix

glm::vec3 camera_pos;

float initialPaddleZ;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int x, int y);
void mouseButton(int button, int state, int x, int y);

float oldCursor[2], newCursor[2];
bool mouseStoreFlag = false;

//--Resource management
bool initialize();
void loadTexture(const char* name, GLuint &textID);
void LightingUpdate();


//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Main
int main(int argc, char **argv) {
    //Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    Lighting.Init();
    //Name and create the Window
    glutCreateWindow("Lighting");

    //initialize textures
    char blueTexture[] = "../textures/blue.jpg";
    char greenTexture[] = "../textures/green.jpg";
    char tanTexture[] = "../textures/tan.jpg";

    //load textures with Magick++
    Magick::InitializeMagick(*argv);
    loadTexture(blueTexture, blue);
    loadTexture(greenTexture, green);
    loadTexture(tanTexture, tanTex);



    GLenum status = glewInit(); //check glut status
    if( status != GLEW_OK){
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    //Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);    // Called when its time to display
    glutReshapeFunc(reshape);   // Called if the window is resized
    glutIdleFunc(update);       // Called if there is nothing else to do
    glutKeyboardFunc(keyboard); // Called if there is keyboard input
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouse);  // Constantly tracks cursor while clicked
    //Initialize all of our resources
    shaderManager = new ShaderManager();

    bool init = initialize(); //initialize shaders and load models  

    //Initialize all of our meshes
    tempMesh.initializeMesh("sphere");
    tempMesh.initial_bound = meshManager.getBounds("sphere");   //set current position
    tempMesh.current_position = tempMesh.initial_bound;       //set current position
    sphere = tempMesh;

    if(init) 
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    return 0;
}

//--Implementations
void render()
{

    //--Render the scene

    //clear the screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    /////////////////
    /* Set up sphere */
    /////////////////
    
    //get model view matrix
    mv = view * sphere.model;
    
    //enable the shader program
    shaderManager->useProgram();

/*
    vec3 Ls = vec3(1.0, 1.0, 1.0); // white specular colour
    vec3 Ld = vec3(0.7, 0.7, 0.7); // dull white diffuse light colour
    vec3 La = vec3(0.2, 0.2, 0.2); // grey ambient colour


    GLfloat specularVec[3];
    GLfloat ambientVec[3];
    GLfloat diffuseVec[3];
    GLfloat lightVec[3];
*/

     glUniform3fv(ambient_data, 1, Lighting.ambientVec);
     glUniform3fv(diffuse_data, 1, Lighting.diffuseVec);
     glUniform3fv(specular_data, 1, Lighting.specularVec);
     glUniform3fv(light_position, 1, Lighting.lightVec);


    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  
    
    // Calculate surface normal vector
   // normalVec.
    
   // glUniform3f(surface_normal, 1, normalVec);

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(scene_normal);
    glEnableVertexAttribArray(textureCoord);



    //set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, green);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("sphere"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset


    glVertexAttribPointer(  scene_normal, 
                            3 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,normal) );
               
                       
    glVertexAttribPointer(  textureCoord, 
                            2 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,textureCoords) );
             

  
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("sphere"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisableVertexAttribArray(scene_normal);
    

    glDisable(GL_TEXTURE_2D);

    //swap the buffers
    glutSwapBuffers();
}

void update() {

    //total time
    float dt = getDT(); // if you have anything moving, use dt.
    
    LightingUpdate();

    //axes = qt.getAxis();
    sphere.model=glm::translate(glm::mat4(1.0f), glm::vec3( sphere.offset.x,
                                                            sphere.offset.y,
                                                            sphere.offset.z));

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
    projection = glm::perspective(90.0f, float(w)/float(h), 0.01f, 100.0f);

}

//handle keyboard input
void keyboard(unsigned char key, int x_pos, int y_pos)
{
    

    switch(key)
    {
        case 27: //escape
            exit(0);
            break;
            
        case 'S':
        case 's':
            if(!specularOn)
            {
                specularOn = true;
            }
            else
            {
                specularOn = false;
            }
            break;
            
        case 'A':
        case 'a':
            if(!ambientOn)
            {
                ambientOn = true;
            }
            else
            {
                ambientOn = false;
            }
            break;
        case 'D':
        case 'd':
            if(!diffuseOn)
            {
                diffuseOn = true;
            }
            else
            {
                diffuseOn = false;
            }
            break;            
          
    }
}


void mouse( int x, int y)
{
/*
    float normx = (2.0f * x) / w - 1.0f;
    float normy = -((2.0f * y) / h - 1.0f);
    float normz = 1.0f;

    glm::vec4 norm = glm::vec4(normx, normy, normz, 1.0f);
    glm::vec4 normObjectDepth = glm::vec4(normx, normy, normz, 1.0f); // finding cursor project ray to where object is

    norm = glm::inverse(projection)*norm;
    
    glm::mat4 invertedView = glm::inverse(view);
    
    norm *= 100.0f; // projection depth
    normObjectDepth *= m_Physics.simulationPaddle->getCenterOfMassPosition().getZ();// depth of paddle
    
    norm = invertedView * norm;
    norm /= norm.w;
    
    normObjectDepth = invertedView * normObjectDepth;
    normObjectDepth /= normObjectDepth.w;
    
    glm::vec3 ray_world = glm::vec3(norm);
    ray_object_depth = glm::vec3(normObjectDepth);
    
    // get camera position
    GLdouble mdl[16];
    float camera_org[3];
    glGetDoublev(GL_MODELVIEW_MATRIX, mdl);
    camera_org[0] = -(mdl[0] * mdl[12] + mdl[1] * mdl[13] + mdl[2] * mdl[14]);
    camera_org[1] = -(mdl[4] * mdl[12] + mdl[5] * mdl[13] + mdl[6] * mdl[14]);
    camera_org[2] = -(mdl[8] * mdl[12] + mdl[9] * mdl[13] + mdl[10] * mdl[14]);

    camera_pos = glm::vec3(camera_org[0], camera_org[1], camera_org[2]);
    
    //forceVector = glm::normalize(ray_world);
    rayDestination = btVector3(ray_world.x, ray_world.y, ray_world.z);

    rayTraceResult = m_Physics.rayHit( camera_pos, ray_world, hitPointWorld );
    
    glutPostRedisplay();
    * 
    * */
}

void mouseButton(int button, int state, int x, int y)
{/*
    // detect button release
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP && rayTraceResult == false && picked == true )
    {
        picked = false;
    }
   */ 
}

bool initialize()
{
    // Initialize basic geometry and shaders for this example
    string sphereObj  = "../objectFiles/sphere.obj";

    //load paddle
    if(!meshManager.loadModel(sphereObj, "sphere")) 
    {
        cout <<"[ERROR] " << sphereObj  << " Model could not be loaded" << endl;
        return false;
    }
    
    //Shader Sources
    char vertexshader[] = "../src/VertexShader.txt";
    char fragmentshader[] = "../src/FragmentShader.txt";

    //initialize shaders
    if(!shaderManager->loadVertexShader(vertexshader))
    {   
        return false;
    }
    if(!shaderManager->loadFragmentShader(fragmentshader))
    {    
        return false;
    }
    
    //link programs
    shaderManager->linkProgramObject();

    //locate attributes
    scene_mv = shaderManager->getUniformAttributeLocation("ModelView");
    if(scene_mv == -1) 
    {
        std::cerr << "[F] MODELVIEW MATRIX NOT FOUND " << std::endl;
        return false;
    }

    scene_p = shaderManager->getUniformAttributeLocation("Projection");
    if(scene_p == -1) 
    {
        std::cerr << "[F] PROJECTION MATRIX NOT FOUND " << std::endl;
        return false;
    }

    scene_position = shaderManager->getAttributeLocation("v_position");
    if(scene_position == -1) 
    {
        std::cerr << "[F] POSITION ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    
     textureCoord = shaderManager->getAttributeLocation("v_texCoord");
    if(textureCoord == -1) 
    {
        std::cerr << "[F] V_textureCoord ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    scene_normal = shaderManager->getAttributeLocation("v_normal");
    if(scene_normal == -1) 
    {
        std::cerr << "[F] v_normal ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }   

    light_position = shaderManager->getUniformAttributeLocation("lightPosition");
    if(light_position == -1) 
    {
        std::cerr << "[F] LIGHTPOSITION ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    
    diffuse_data = shaderManager->getUniformAttributeLocation("Diffuse_Data");
    if(diffuse_data == -1) 
    {
        std::cerr << "[F] DIFFUSE ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    
    specular_data = shaderManager->getUniformAttributeLocation("Specular_Data");
    if(specular_data == -1) 
    {
        std::cerr << "[F] SPECULAR ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    ambient_data = shaderManager->getUniformAttributeLocation("ambient");
    if(ambient_data == -1) 
    {
        std::cerr << "[F] AMBIENT ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    TexUnit= shaderManager->getUniformAttributeLocation("texUnit");
    if(TexUnit == -1) 
    {
        std::cerr << "[F] TEXUNIT ATTRIBUTE NOT FOUND " << std::endl;
        //return false;
    }


    //--Init the view and projection matrices
    view = glm::lookAt( glm::vec3(0.0, 2.0, 2.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0),   //Focus point
                        glm::vec3(0.0, 1.0, 0.0));  //Positive Y is up

    projection = glm::perspective( 80.0f, float(w)/float(h), 0.01f, 100.0f);

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_SMOOTH);
    glDepthFunc(GL_LESS);

    return true;
}
//load an image for texture
void loadTexture(const char* name, GLuint &textID) 
{
    Magick::Image* myImage = new Magick::Image( name );
    Magick::Blob myBlob;

    myImage->write(&myBlob, "RGBA");
    
    glGenTextures(1, &textID);
    glBindTexture(GL_TEXTURE_2D, textID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //or GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //or GL_NEAREST
    glTexImage2D(GL_TEXTURE_2D, 
                 0, 
                 GL_RGBA,
                 myImage->columns(),
                 myImage->rows(), 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, myBlob.data());    
}

void LightingUpdate()
{
    if(!specularOn)
    {
        for(int i=0; i<3; i++)
            Lighting.specularVec[i]=0.0;
    }
    else
    {
        for(int i=0; i<3; i++)
            Lighting.specularVec[i]=1.0;
    }

    if(!ambientOn)
    {
        for(int i=0; i<3; i++)
            Lighting.ambientVec[i]=0.0;
    }
    else
    {
        for(int i=0; i<3; i++)
            Lighting.ambientVec[i]=0.2;
    }


    if(!diffuseOn)
    {
        for(int i=0; i<3; i++)
            Lighting.diffuseVec[i]=0.0;
    }
    else
    {
        for(int i=0; i<3; i++)
            Lighting.diffuseVec[i]=0.7;
    }



    
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
