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


#include "MeshManager.h"    //for loading models
#include "ShaderManager.h"  //for managing shaders
#include "Physics.h"        //physics

//CONSTS used for user interactions
int w = 1024, h = 1024;         // Window size
int MOUSE_X = 0, MOUSE_Y = 0;   //used for mouse interface
float DELTA_X_CHANGE = 0.0, DELTA_Y_CHANGE = 0.0; //for mouse and keyboard interaction
float DELTA_X_CHANGE_ai = 0.0, DELTA_Y_CHANGE_ai = 33.0;
double X_CHANGE = 0.05, Y_CHANGE = 0.05; //step change for paddle using keyboard input
int NUMBER_OF_WINS;
bool paused = false;

int AmbientDefault = 2.8;

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
    specularVec[0] = 2.0;
    specularVec[1] = 2.0;
    specularVec[2] = 2.0;
    
    ambientVec[0] = AmbientDefault;
    ambientVec[1] = AmbientDefault;
    ambientVec[2] = AmbientDefault;
    
    diffuseVec[0] = 1.8;
    diffuseVec[1] = 1.8;
    diffuseVec[2] = 1.8;
    
    lightVec[0] = 10.0;
    lightVec[1] = 10.0;
    lightVec[2] = 20.0;
}


//--Managers
MeshManager meshManager;
ShaderManager *shaderManager;
ShaderManager *shaderManager_holes; //three kinds of holes: start, end, trap
int holeType; //used for coloring holes
int level; //what the current difficulty level is
vector<int> numOfHoles; //used for game levels

//--Textures Handles
GLuint black_marble, white_marble, metal, black_solid, red_solid, green_solid;
GLint textureCoord;
GLint textureUnit;

//--Shader Variables
GLint scene_mv;
GLint scene_p;
GLint scene_position;
GLint scene_normal;

//--Meshes
Mesh boardMesh;
Mesh sphereMesh;

vector<Mesh> myMaze; //maze
vector< vector<bool> > maze; //boolean array
vector<Maze_Holes> mazeHoles; //first entry is the start, last entry is the ends

//--Transform matrices
glm::mat4 view;         //world->eye
glm::mat4 projection;   //eye->clip
glm::mat4 mvp;          //premultiplied mvp
glm::mat4 mv;           //model-view matrix

//--Attribute locations
GLint loc_position;
GLint loc_position_sphere;
vector<GLint> loc_positions(3); //loc positions
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader
GLint loc_mvpmat_sphere;
vector<GLint> loc_mvpmats(3); //three mvpmats for the maze holes

// --Lighting
GLint light_position;////////////// ADD
GLint diffuse_data;////////////// ADD
GLint specular_data;////////////// ADD
GLint ambient_data;////////////// ADD

Light Lighting;////////////// ADD

//--Physics handle
Physics myPhysics;
bool initPhysics = true;


//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);
void mouseOnTheMove(int x, int y);

//--Menus
void mainMenu(int id);
void startMenu(int id);
void sensitivityMenu(int id);
void settingsMenu(int id);
void ambientLightMenu(int id);
void difficultyLevel(int id);

//--Resource Management
bool initialize();
void buildMaze(int xSize, int ySize);
void ballSetStart();
void createMenus() ;
void loadTexture(const char* name, GLuint &textID);

//--Game Functions
void keyOperations(float dt);
void updateCamera(float dt);
void defaultCamera();
void inGoal();
void updateAI();
void pauseGame();
void resetPuck();
void resetGame();

//--Random Time Things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Main
int main(int argc, char **argv) 
{
    //Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
        
    Lighting.Init();////////////// ADD

    // Name and create the Window
    glutCreateWindow("LABYRINTH PROGRAM");

    //temp Mesh model
    Mesh tempMesh;
    
    //add the number of holes values
    numOfHoles.push_back(1);
    numOfHoles.push_back(3);
    numOfHoles.push_back(6);
    level = 0;

    //initialize textures
    char metal_file[] = "../textures/metal.png";
    char white_marble_file[] = "../textures/MarbleWhite.png";
    char black_marble_file[] = "../textures/gold.png";
    char red_solid_file[] = "../textures/red.png";
    char green_solid_file[] = "../textures/green.png";
    char black_solid_file[] = "../textures/black.png";

    //load textures
    Magick::InitializeMagick(*argv);
    
    loadTexture(metal_file, metal);
    loadTexture(black_marble_file, black_marble);
    loadTexture(white_marble_file, white_marble);
    loadTexture(red_solid_file, red_solid);
    loadTexture(black_solid_file, black_solid);
    loadTexture(green_solid_file, green_solid);

    GLenum status = glewInit(); //check glut status
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    //Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);    // Called when its time to display
    glutReshapeFunc(reshape);   // Called if the window is resized
    glutIdleFunc(update);       // Called if there is nothing else to do
    glutKeyboardFunc(keyboard); // Called if there is keyboard input
    glutMouseFunc(mouse);
    glutMotionFunc(mouseOnTheMove);

    //Initialize all of our resources
    shaderManager = new ShaderManager();

    bool init = initialize(); //initialize shaders and load models

    //build mesh objects
    tempMesh.initialize("board");
    tempMesh.initial_bound = meshManager.getBounds("board");
    tempMesh.current_position = tempMesh.initial_bound;
    boardMesh = tempMesh;

    tempMesh.initialize("sphere");
    tempMesh.initial_bound = meshManager.getBounds("sphere");
    tempMesh.current_position = tempMesh.initial_bound;
    sphereMesh = tempMesh;


    //initialize maze
    buildMaze(  abs(boardMesh.initial_bound.second.z - boardMesh.initial_bound.first.z) - 1,
                abs(boardMesh.initial_bound.second.x - boardMesh.initial_bound.first.x) - 1);
    
    //translate the ball to the start position
    ballSetStart();
    
    //initialize menus
    createMenus();

    if(init) 
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    return 0;
}

    
void render()
{
    //clear the screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //////////////////
    /* SET UP TABLE */
    //////////////////

    //get mv matrix
    mv = view * boardMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    glUniform3fv(ambient_data, 1, Lighting.ambientVec);
    glUniform3fv(diffuse_data, 1, Lighting.diffuseVec);
    glUniform3fv(specular_data, 1, Lighting.specularVec);
    glUniform3fv(light_position, 1, Lighting.lightVec);


    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload light information
    //-----------------------------

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(scene_normal);
    glEnableVertexAttribArray(textureCoord);

    //set the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black_marble);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("board"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
    
    glVertexAttribPointer(  scene_normal, 
                            3,
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
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("board"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(scene_normal);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);
    
    
    /////////////////
    /* SET UP BALL */
    /////////////////

    //get mv matrix
    mv = view * boardMesh.model * sphereMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload light information
    //-----------------------------

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(scene_normal);
    glEnableVertexAttribArray(textureCoord);

    //set the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, white_marble);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("sphere"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
    
    glVertexAttribPointer(  scene_normal, 
                            3,
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
    glDisableVertexAttribArray(scene_normal);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);

    /////////////////
    /* SET UP MAZE */
    /////////////////

    for(auto i: myMaze)
    {
        //get mv matrix
        mv = view * boardMesh.model *i.model;

        //enable the shader program
        shaderManager->useProgram();

        //upload the matrix to the shader
        glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
        glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

        //upload light information
        //-----------------------------

        //set up the Vertex Buffer Object so it can be drawn
        glEnableVertexAttribArray(scene_position);
        glEnableVertexAttribArray(scene_normal);
        glEnableVertexAttribArray(textureCoord);

        //set the textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, black_marble);
        glUniform1i(textureUnit, 0);

        glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("maze wall"));

        //set pointers into the vbo for each of the attributes(position and normal)
        glVertexAttribPointer(  scene_position,     //location of attribute
                                3,                  //number of elements
                                GL_FLOAT,           //type
                                GL_FALSE,           //normalized?
                                sizeof(vertex),     //stride
                                0);                 //offset
        
        glVertexAttribPointer(  scene_normal, 
                                3,
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
                                
        glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("maze wall"));//mode, starting index, count

        //clean up
        glDisableVertexAttribArray(scene_position);
        glDisableVertexAttribArray(scene_normal);
        glDisableVertexAttribArray(textureCoord);
        glDisable(GL_TEXTURE_2D);       
    } 
        

    /////////////////
    /* SET UP HOLES*/
    /////////////////

    for(auto i: mazeHoles)
    {
        //get mv matrix
        mv = view * boardMesh.model *i.model;

        //enable the shader program
        if(i.first)
        {
            holeType = 0;
        }
        else if(i.last)
        {
            holeType = 1;
        }
        else
        {
            holeType = 2;
        }
        //enable shader program
        shaderManager->useProgram();

    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  

        //set up the Vertex Buffer Object so it can be drawn
        glEnableVertexAttribArray(scene_position);
       // glEnableVertexAttribArray(loc_positions[holeType]);
        glEnableVertexAttribArray(scene_normal);
        glEnableVertexAttribArray(textureCoord);
        
        //set the textures
        glActiveTexture(GL_TEXTURE0);
        
        if(holeType == 0)
        {
            glBindTexture(GL_TEXTURE_2D, green_solid);    
        }
        else if(holeType == 1)
        {
            glBindTexture(GL_TEXTURE_2D, red_solid);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, black_solid);
        }
        
        glUniform1i(textureUnit, 0);
        glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("hole"));

        //set pointers into the vbo for each of the attributes(position and color)
        glVertexAttribPointer(  scene_position,
                                3,
                                GL_FLOAT, 
                                GL_FALSE, 
                                sizeof(vertex), 
                                0);
        glVertexAttribPointer(  scene_normal, 
                                3,
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
                                
        glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("hole"));//mode, starting index, count
        
        //clean up
        glDisableVertexAttribArray(scene_position);  
        glDisableVertexAttribArray(textureCoord);  
        glDisableVertexAttribArray(scene_normal);
    }
      
    //swap the buffers
    glutSwapBuffers();
}
    
    
void update() 
{
    //total time
    float dt = getDT();// if you have anything moving, use dt.

    //update board
    boardMesh.model=glm::rotate(glm::mat4(1.0f),float(DELTA_X_CHANGE*180/3.1415), glm::vec3(0,0,1));
    boardMesh.model=glm::rotate(boardMesh.model,float(DELTA_Y_CHANGE*180/3.1415), glm::vec3(1,0,0));


    //update sphere
    //check for collisions
    if(sphereMesh.holeTopFlag)
    {
        sphereMesh.checkBound(mazeHoles); //holes
    }
    else //if the ball is on a hole
    ballSetStart();

    if(paused == false) 
    {
        //update sphere based on collisions
        sphereMesh.updateBounds(dt); //update forces

        myPhysics.simulate(sphereMesh);
        btTransform trans;
        myPhysics.simulationBall->getMotionState()->getWorldTransform(trans);

        //update the x,y, and z
        sphereMesh.offset.x = trans.getOrigin().getX();
        sphereMesh.offset.y = trans.getOrigin().getY();
        sphereMesh.offset.z = trans.getOrigin().getZ();

        sphereMesh.model=glm::translate(glm::mat4(1.0f), glm::vec3(sphereMesh.offset.x, sphereMesh.offset.y, sphereMesh.offset.z));
        sphereMesh.model=glm::rotate(sphereMesh.model,float(DELTA_X_CHANGE*180*dt/3.1415), glm::vec3(0,0,1));
        sphereMesh.model=glm::rotate(sphereMesh.model,float(DELTA_Y_CHANGE*180*dt/3.1415), glm::vec3(1,0,0));
     }

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

void keyboard(unsigned char key, int x_pos, int y_pos) 
{

    // Handle keyboard input
    if(key == 27) { //ESC
        exit(0);
    }
    else if(key == 108) { //'l' -- about the y axis
        DELTA_X_CHANGE += X_CHANGE;
    }
    else if(key == 114) { //'r' -- about the y axis
        DELTA_X_CHANGE -= X_CHANGE;
    }
    else if(key == 117 ) { //'u' -- about the x axis
        DELTA_Y_CHANGE += Y_CHANGE;
    }
    else if(key == 100) { //'d' -- about the x axis
        DELTA_Y_CHANGE -= Y_CHANGE;
    }
}


void mouse(int button, int state, int x, int y) 
{
       MOUSE_X = x;
       MOUSE_Y = y;
}

void mouseOnTheMove(int x, int y) 
{

       if(MOUSE_X > x)
          DELTA_X_CHANGE -= abs(float(MOUSE_X - x))/300.0;
       else
          DELTA_X_CHANGE += abs(float(MOUSE_X - x))/300.0;

       if(MOUSE_Y > y)
          DELTA_Y_CHANGE += abs(float(MOUSE_Y - y))/300.0;
       else
          DELTA_Y_CHANGE -= abs(float(MOUSE_Y - y))/300.0;
      MOUSE_Y = y;
      MOUSE_X = x;
}


bool initialize()
{
    // Initialize basic geometry and shaders for this example
    string ballObj  = "../objectFiles/ball.obj";
    string boardObj = "../objectFiles/board.obj";
    string mazeObj  = "../objectFiles/maze.obj";
    string holeObj  = "../objectFiles/hole.obj";

    //load board
    if(!meshManager.loadModel(boardObj, "board")) 
    {
        cout <<"[ERROR] " << boardObj  << " Model could not be loaded" << endl;
        return false;
    }
    //load ball
    if(!meshManager.loadModel(ballObj, "sphere")) 
    {
        cout << "[ERROR] " << ballObj << " Model could not be loaded" << endl;
        return false;
    }
    //load maze wall
    if(!meshManager.loadModel(mazeObj, "maze wall")) 
    {
        cout << "[ERROR] " << mazeObj << " Model could not be loaded" << endl;
        return false;
    }
    //load holes
    if(!meshManager.loadModel(holeObj, "hole")) 
    {
        cout << "[ERROR] " << holeObj << " Model could not be loaded" << endl;
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

    textureUnit= shaderManager->getUniformAttributeLocation("texUnit");
    if(textureUnit == -1) 
    {
        std::cerr << "[F] TEXUNIT ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    //////////////// ADDED
    light_position = shaderManager->getUniformAttributeLocation("lightPosition");
    if(light_position == -1) 
    {
        std::cerr << "[F] LIGHTPOSITION ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    //////////////// ADDED
    diffuse_data = shaderManager->getUniformAttributeLocation("Diffuse_Data");
    if(diffuse_data == -1) 
    {
        std::cerr << "[F] DIFFUSE ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    //////////////// ADDED
    specular_data = shaderManager->getUniformAttributeLocation("Specular_Data");
    if(specular_data == -1) 
    {
        std::cerr << "[F] SPECULAR ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    //////////////// ADDED
    ambient_data = shaderManager->getUniformAttributeLocation("ambient");
    if(ambient_data == -1) 
    {
        std::cerr << "[F] AMBIENT ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
        
    
    //////////////// ADDED
    light_position = shaderManager->getUniformAttributeLocation("lightPosition");
    if(light_position == -1) 
    {
        std::cerr << "[F] LIGHTPOSITION ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    //////////////// ADDED
    diffuse_data = shaderManager->getUniformAttributeLocation("Diffuse_Data");
    if(diffuse_data == -1) 
    {
        std::cerr << "[F] DIFFUSE ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    //////////////// ADDED
    specular_data = shaderManager->getUniformAttributeLocation("Specular_Data");
    if(specular_data == -1) 
    {
        std::cerr << "[F] SPECULAR ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    //////////////// ADDED
    ambient_data = shaderManager->getUniformAttributeLocation("ambient");
    if(ambient_data == -1) 
    {
        std::cerr << "[F] AMBIENT ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }
    
    //--Init the view and projection matrices
    view = glm::lookAt( glm::vec3(0.0, 18.0, -8.0), //Eye Position 25 -8
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 90.0f, float(w)/float(h), 0.01f, 100.0f);

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_SMOOTH);
    glDepthFunc(GL_LESS);

    return true;
}



void createMenus() 
{

    //the start menu
    int start_menu;
    start_menu = glutCreateMenu(startMenu);
    glutAddMenuEntry("PAUSE", 1);
    glutAddMenuEntry("RESTART", 2);
    glutAddMenuEntry("RESUME", 3);
    glutAddMenuEntry("NEW MAZE", 4);

    //the sensitivity menu
    int _sensitivity = glutCreateMenu(sensitivityMenu);
    glutAddMenuEntry("VERY SENSITIVE", 1);
    glutAddMenuEntry("KIND OF SENSITIVE", 2);
    glutAddMenuEntry("WEAK SAUCE!!!", 3);

    //type of ball stuff
    int _AMBIENT_LIGHT = glutCreateMenu(ambientLightMenu);
    glutAddMenuEntry("ON", 1);
    glutAddMenuEntry("OFF", 2);


    int _difficulty = glutCreateMenu(difficultyLevel);
    glutAddMenuEntry("BEASTLY", 1);
    glutAddMenuEntry("NOT SO BEASTLY", 2);
    glutAddMenuEntry("EASY PEASY", 3);

    //the setting menu
    int settings;
    settings = glutCreateMenu(settingsMenu);
    glutAddSubMenu("SENSITIVITY", _sensitivity);
    glutAddSubMenu("AMBIENT LIGHT", _AMBIENT_LIGHT);
    glutAddSubMenu("DIFFICULTY", _difficulty);

    glutCreateMenu(mainMenu);
    glutAddSubMenu("START", start_menu);
    glutAddSubMenu("SETTINGS", settings);
    glutAddMenuEntry("QUIT", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void ambientLightMenu(int id) 
{
   switch(id) 
   {
        case 1:
            Lighting.ambientVec[0] = AmbientDefault;
            Lighting.ambientVec[1] = AmbientDefault;
            Lighting.ambientVec[2] = AmbientDefault;
            break;
        case 2:
            Lighting.ambientVec[0] = 0.5;
            Lighting.ambientVec[1] = 0.5;
            Lighting.ambientVec[2] = 0.5;
            break;
   }
}

void difficultyLevel(int id) 
{
    switch(id) 
    {
        case 1: //beastly
            level = 2;//set level
            break;
        case 2: //not so beastly
            level = 1;
            break;
        case 3: //easy peasy
            level = 0;
            break;
    }
    NUMBER_OF_WINS = 0; //reset the number of wins
    buildMaze(  abs(boardMesh.initial_bound.second.z - boardMesh.initial_bound.first.z) - 1, 
                abs(boardMesh.initial_bound.second.x - boardMesh.initial_bound.first.x) - 1);
    ballSetStart();
}

void sensitivityMenu(int id) 
{
    switch(id) 
    {
        case 1: //very sensitive
            sphereMesh.sensitivity = glm::vec4(100.0f, 0.0f, 100.0f, 0.0f);
            break;
        case 2: //kind of
            sphereMesh.sensitivity = glm::vec4(50.0f, 0.0f, 50.0f, 0.0f);
            //glutIdleFunc(update);
            break;
        case 3: //very little
            sphereMesh.sensitivity = glm::vec4(10.0f, 0.0f, 10.0f, 0.0f);
            break;
    }
}

void settingsMenu(int id) 
{
}

void mainMenu(int id) 
{
    switch(id) 
    {
        case 1:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void startMenu(int id) 
{
   switch(id) 
   {
        case 1: //pause
            paused = true;
            break;
        case 2: //restart
            ballSetStart();
            break;
        case 3://resume
            paused = false;
            break;
        case 4://new maze
            buildMaze(  abs(boardMesh.initial_bound.second.z - boardMesh.initial_bound.first.z) - 1, 
                        abs(boardMesh.initial_bound.second.x - boardMesh.initial_bound.first.x) - 1);
            ballSetStart();
        break;
   }
}

void buildMaze(int xSize, int ySize) 
{
    srand(time(0));
    list < pair < int, int> > drillers;

    maze.resize(xSize);
    for(unsigned int y = 0; y < maze.size(); y++) 
    {
        maze[y].resize(ySize);
        for(unsigned int x = 0; x < maze[y].size(); x++) 
        {
           maze[y][x] = false;
        }
    }
   
   drillers.push_back(make_pair(xSize/2,ySize/2));

   while(drillers.size() > 0)  
   {
        list < pair < int, int> >::iterator m,_m,temp;
        m=drillers.begin();
        _m=drillers.end();
        while (m!=_m) 
        {
            bool remove_driller=false;
            switch(rand()%4)  
            {
                case 0:
                    (*m).second-=2;
                    if((*m).second < 0 || maze[(*m).second][(*m).first]) 
                    {
                        remove_driller=true;
                        break;
                    }
                    
                    maze[(*m).second+1][(*m).first]=true;
                    break;
                    
                case 1:
                    (*m).second+=2;
                    if((*m).second>=ySize || maze[(*m).second][(*m).first]) 
                    {
                        remove_driller=true;
                        break;
                    }
                    
                    maze[(*m).second-1][(*m).first]=true;
                    break;
                    
                case 2:
                    (*m).first-=2;
                    if((*m).first<0 || maze[(*m).second][(*m).first])
                    {
                        remove_driller=true;
                        break;
                    }
                    
                    maze[(*m).second][(*m).first+1]=true;
                    break;
                    
                case 3:
                    (*m).first+=2;

                    if((*m).first>=xSize || maze[(*m).second][(*m).first]) 
                    {
                        remove_driller=true;
                        break;
                    }
                    
                    maze[(*m).second][(*m).first-1]=true;
                    break;
            }
            
            if (remove_driller)
            {
                m = drillers.erase(m);
            }
            else 
            {
                drillers.push_back(make_pair((*m).first,(*m).second));
                // uncomment the line below to make the maze easier
                //if (rand()%2)
                drillers.push_back(make_pair((*m).first,(*m).second));

                maze[(*m).second][(*m).first]=true;
                ++m;
            }
        }
    }


    //build maze, select start and end positions, and random hole placement
    Mesh mesh;
    Maze_Holes temp_holes;
    glm::vec2 last_position = glm::vec2(0.0f);

    mesh.initialize("maze wall");
    mesh.initial_bound = meshManager.getBounds("maze wall");
    mesh.current_position = mesh.initial_bound;

    temp_holes.init();
    temp_holes.initial_bound = meshManager.getBounds("hole");
    temp_holes.current_position = temp_holes.initial_bound;
    myMaze.clear();
    mazeHoles.clear();
    
    for(unsigned int i = 0; i < maze.size(); i ++) //row
    {
        for(unsigned int j = 0; j < maze[i].size(); j ++) //column
        { 
            if(maze[i][j] == false) //this is blocked
            { 
                //set the offset
                mesh.offset.x = ( (float)i - (boardMesh.initial_bound.second.x - 0.5f));
                mesh.offset.z = ((float)j - (boardMesh.initial_bound.second.z - 1.2f) );
                mesh.offset.y = -0.7f; //floor of the board -- could use boardMesh.initial_bound.first.y
                //translate the matrix
                mesh.model=glm::translate(glm::mat4(1.0f), glm::vec3( mesh.offset.x, mesh.offset.y, mesh.offset.z) );
                //update the current positions
                mesh.current_position.first = mesh.initial_bound.first + mesh.offset;
                mesh.current_position.second = mesh.initial_bound.second + mesh.offset;
                //push onto the vector
                myMaze.push_back(mesh);
             }
             else //otherwise set the start, end, and random hole positions
             {
                //save position to determine end location
                last_position.x = i;
                last_position.y = j;

                //if this is the first path location or if this will be a random position
                if(mazeHoles.size() == 0 || rand()%20 < numOfHoles[level]) 
                {
                    temp_holes.offset.x = ( (float)i - (boardMesh.initial_bound.second.x - 0.5f));
                    temp_holes.offset.z = ((float)j - (boardMesh.initial_bound.second.z - 1.2f) );
                    temp_holes.offset.y = -.65f;
                    temp_holes.model=glm::translate(glm::mat4(1.0f), glm::vec3( temp_holes.offset.x, temp_holes.offset.y, temp_holes.offset.z) );
                    
                    //update the current positions
                    temp_holes.current_position.first = temp_holes.initial_bound.first + temp_holes.offset;
                    temp_holes.current_position.second = temp_holes.initial_bound.second + temp_holes.offset;

                    if(mazeHoles.size() == 0) //if this is the first one, set the color to green
                    {
                        temp_holes.type = 1.0;
                        temp_holes.first = true;
                    }
                    else
                    {
                        temp_holes.type = 3.0;
                    }
                    
                    //push onto the vector
                    mazeHoles.push_back(temp_holes);
                    temp_holes.first = false;
                }
            }
        }
    }
    
    //set the end position
    temp_holes.offset.x = ( (float)last_position.x - (boardMesh.initial_bound.second.x - 0.5f));
    temp_holes.offset.z = ((float)last_position.y - (boardMesh.initial_bound.second.z - 1.2f) );
    temp_holes.offset.y = -.65f;
    temp_holes.model=glm::translate(glm::mat4(1.0f), glm::vec3( temp_holes.offset.x, temp_holes.offset.y, temp_holes.offset.z) );
    
    //update the current positions
    temp_holes.current_position.first = temp_holes.initial_bound.first + temp_holes.offset;
    temp_holes.current_position.second = temp_holes.initial_bound.second + temp_holes.offset;
    temp_holes.type = 2.0f;
    temp_holes.last =  true;
    mazeHoles.push_back(temp_holes);
}

void ballSetStart() 
{
    if(sphereMesh.whichHole.last) 
    {
         cout << "YOU WON!!!!!" << endl;
         NUMBER_OF_WINS ++;
         buildMaze( abs(boardMesh.initial_bound.second.z - boardMesh.initial_bound.first.z) - 1, 
                    abs(boardMesh.initial_bound.second.x - boardMesh.initial_bound.first.x) - 1);
    }
    else
    {
        cout << "AWWWWW!! TOO BAD!!!!!!" << endl;
    }
    cout << "NUMBER OF WINS: " << NUMBER_OF_WINS << endl;

    MOUSE_X = 0; MOUSE_Y = 0;
    DELTA_X_CHANGE = 0.0; DELTA_Y_CHANGE = 0.0; //for mouse movement stuff
    X_CHANGE = 0.05; Y_CHANGE = 0.05;

    boardMesh.initialize("board");
    boardMesh.initial_bound = meshManager.getBounds("board");
    boardMesh.current_position = boardMesh.initial_bound;

    sphereMesh.initialize("sphere");
    sphereMesh.initial_bound = meshManager.getBounds("sphere");
    sphereMesh.current_position = sphereMesh.initial_bound;

    //set the location of the ball on top of the start hole
    sphereMesh.offset.x = mazeHoles[0].offset.x;
    sphereMesh.offset.z = mazeHoles[0].offset.z;
    sphereMesh.offset.y = 3.0f;

    //translate to the start position and update the current positions
    sphereMesh.model=glm::translate(glm::mat4(1.0f), glm::vec3( sphereMesh.offset.x, sphereMesh.offset.y, sphereMesh.offset.z));
    sphereMesh.current_position.first = sphereMesh.initial_bound.first + sphereMesh.offset;
    sphereMesh.current_position.second = sphereMesh.initial_bound.second + sphereMesh.offset;

    //set up the board and ball in the simulated physics world
    if(initPhysics == true) 
    {
        myPhysics.makeBoard(boardMesh);
        myPhysics.makeBall(sphereMesh);
        myPhysics.makeWalls(myMaze);
        initPhysics = false;
    }
    else 
    {
        myPhysics.resetMaze(sphereMesh, myMaze);
    }
}


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


//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}








