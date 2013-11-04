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
float DELTA_X_CHANGE = 0.0, DELTA_Y_CHANGE = -33.0; //for mouse and keyboard interaction
float DELTA_X_CHANGE_ai = 0.0, DELTA_Y_CHANGE_ai = 33.0;
double X_CHANGE = 0.5, Y_CHANGE = 0.5; //step change for paddle using keyboard input
int PLAYER_SCORE = 0, AI_SCORE = 0, WIN_SCORE = 7;

//--Camera Variables
float upX, upY, upZ;
float eyeX, eyeY, eyeZ;
float theta, phi, radius;

//--Game Control Variables
GLuint ai_control = 0;
GLfloat ai_paddle_speed = 0.15;
float message_time = 0;
float message_x = -100;
std::string message = "";

//--Keyboard Variables
bool* specialKeyStates = new bool[255]; //used for special keypresses
bool* keyStates = new bool[255];
int sensitivity = 25;
bool pauseGame = false;

// Paddle control variables
glm::vec3 forceVector = glm::vec3(0,0,0);
btVector3 hitPointWorld = btVector3(0,0,0);
glm::vec3 camera_pos;
bool rayTraceResult = false;
bool picked = false;
btVector3 rayDestination;
glm::vec3 ray_object_depth;

//--Managers
MeshManager meshManager;
ShaderManager *shaderManager;

//--Textures Handles
GLuint green;
GLuint blue;
GLuint black;
GLint textureCoord;
GLint textureUnit;
vector<GLuint> themes;
int THEME = 2;

//--Shader Variables
GLint scene_mv;
GLint scene_p;
GLint scene_position;

//--Meshes
Mesh puckMesh;
Mesh tableMesh;
Mesh paddlePlayerMesh;
Mesh paddleAIMesh;
Mesh groundMesh;
Mesh wallMesh;
Mesh tempMesh;

//--Transform matrices
glm::mat4 view;         //world->eye
glm::mat4 projection;   //eye->clip
glm::mat4 mv;           //model-view matrix

//--Physics handle
btTransform translate;
Physics myPhysics;

//--GLUT Callbacks
void render();  //render the scene
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void keyboardUp(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);
void mouseOnTheMove(int x, int y);
void special(int key, int x_pos, int y_pos);
void specialUp(int key, int x_pos, int y_pos);

//--Resource Management
bool initialize();
void loadTexture(const char* name, GLuint &textID);
void inGoal();
void createMenus();
void mainMenu(int id);
void themesMenu(int id);
void lightMenu(int id);
void aiLevelMenu(int id);
void updateAI();

//--Helper functions
void keyOperations(float dt);
void updateCamera(float dt);
void defaultCamera();
void resetPuck();
void resetGame();
void pauseTheGame();


//--Random Time Things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Main
int main(int argc, char **argv) 
{
    //Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    //Name and create the Window
    glutCreateWindow("Air Hockey");

    //initialize textures
    string themeFileName[] = {  "...",
                                "...",
                                "..." };
                                
    char blueTexture[] = "../textures/blue.jpg";
    char greenTexture[] = "../textures/green.jpg";
    char blackTexture[] = "../textures/black.jpg";

    //load textures
    Magick::InitializeMagick(*argv);
    loadTexture(blueTexture, blue);
    loadTexture(greenTexture, green);
    loadTexture(blackTexture, black);

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
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseOnTheMove);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    
    
    //Initialize all of our resources
    shaderManager = new ShaderManager();

    bool init = initialize(); //initialize shaders and load models  

    //Initialize all of our meshes
    tempMesh.initializeMesh("table");
    tempMesh.initialPos = meshManager.getBounds("table");  
    tempMesh.currentPos = tempMesh.initialPos;        
    tableMesh = tempMesh;
    
    tempMesh.initializeMesh("walls");
    tempMesh.initialPos = meshManager.getBounds("walls"); 
    tempMesh.currentPos = tempMesh.initialPos;         
    wallMesh = tempMesh;

    tempMesh.initializeMesh("ground");
    tempMesh.initialPos = meshManager.getBounds("ground");
    tempMesh.currentPos = tempMesh.initialPos;
    groundMesh = tempMesh;
    groundMesh.offset.y = -15.0;
    groundMesh.model = glm::translate(glm::mat4(1.0f), glm::vec3(groundMesh.offset.x, groundMesh.offset.y, groundMesh.offset.z));

    tempMesh.initializeMesh("puck");
    tempMesh.initialPos = meshManager.getBounds("puck");
    tempMesh.currentPos = tempMesh.initialPos;
    puckMesh = tempMesh;
    puckMesh.offset.x = tableMesh.currentPos.first.y+0.6;
    puckMesh.offset.z = -5.0;

    tempMesh.initializeMesh("paddle");
    tempMesh.initialPos = meshManager.getBounds("paddle"); 
    tempMesh.currentPos = tempMesh.initialPos;         
    paddlePlayerMesh = tempMesh;
    paddleAIMesh = tempMesh;


    //initialize the physics
    myPhysics.createTable(tableMesh);
    myPhysics.createGround(groundMesh);
    myPhysics.createPuck(puckMesh);
    myPhysics.createPaddle_Player(paddlePlayerMesh);
    myPhysics.createPaddle_AI(paddleAIMesh);
    
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
    //clear the screen
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //////////////////
    /* SET UP TABLE */
    //////////////////

    //get mv matrix
    mv = view * tableMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("table"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
                            
    glVertexAttribPointer(  textureCoord, 
                            2 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,textureCoords) );
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("table"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);
        
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////
    /* SET UP GROUND */
    ///////////////////

    //get mv matrix
    mv = view * groundMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("ground"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
                            
    glVertexAttribPointer(  textureCoord, 
                            2 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,textureCoords) );
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("ground"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);
        
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////
    /* SET UP WALL */
    /////////////////

    //get model view matrix
    mv = view * wallMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, green);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("walls"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
                            
    glVertexAttribPointer(  textureCoord, 
                            2 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,textureCoords) );
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("walls"));//mode, starting index, count
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    
    /////////////////
    /* SET UP PUCK */
    /////////////////
    
    //get model view matrix
    mv = view * tableMesh.model * puckMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, green);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("puck"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
                            
    glVertexAttribPointer(  textureCoord, 
                            2 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,textureCoords) );
                           
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("puck"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);    

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////
    /* SET UP PADDLE */
    ///////////////////
    
    //get model view matrix
    mv = view * tableMesh.model * paddlePlayerMesh.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blue);
    glUniform1i(textureUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("paddle"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer(  scene_position,     //location of attribute
                            3,                  //number of elements
                            GL_FLOAT,           //type
                            GL_FALSE,           //normalized?
                            sizeof(vertex),     //stride
                            0);                 //offset
                            
    glVertexAttribPointer(  textureCoord, 
                            2 , 
                            GL_FLOAT, 
                            GL_FALSE, 
                            sizeof(vertex), 
                            (void*)offsetof(vertex,textureCoords) );
    
    //--DRAW AI PADDLE              
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("paddle"));//mode, starting index, count
    
    //get model view matrix
    mv = view * tableMesh.model * paddlePlayerMesh.model;  
      
    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection
    
    //set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blue);
    glUniform1i(textureUnit, 0);
    
    //--DRAW PLAYER PADDLE
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("paddle"));//mode, starting index, count   
    
    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);
    
    //swap the buffers
    glutSwapBuffers();
}

void update() 
{
    float dt = getDT(); // if you have anything moving, use dt.
    keyOperations(dt/4);
    if(!pauseGame) 
    {
        //AI
        if(ai_control == 0)
        {
		    updateAI();
        }
        
        //--Update the view matrix
        updateCamera(dt/4);
        view = glm::lookAt( glm::vec3(eyeX, eyeY, eyeZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(upX, upY, upZ));

        //update puck and paddle locations
        puckMesh.updateBounds(dt);
        paddlePlayerMesh.updateBounds(dt);
        paddleAIMesh.updateBounds(dt);
        inGoal(); // if goal

        //simulate
        myPhysics.simulate(-1*DELTA_X_CHANGE, DELTA_Y_CHANGE, -1*DELTA_X_CHANGE_ai, DELTA_Y_CHANGE_ai);
        myPhysics.simulate(-1*DELTA_X_CHANGE, DELTA_Y_CHANGE, -1*DELTA_X_CHANGE_ai, DELTA_Y_CHANGE_ai);
        myPhysics.simulate(-1*DELTA_X_CHANGE, DELTA_Y_CHANGE, -1*DELTA_X_CHANGE_ai, DELTA_Y_CHANGE_ai);

        //update puck position
        myPhysics.puck->getMotionState()->getWorldTransform(translate);
        puckMesh.offset.x = translate.getOrigin().getX();
        puckMesh.offset.y = translate.getOrigin().getY();
        puckMesh.offset.z = translate.getOrigin().getZ();
        puckMesh.model=glm::translate(glm::mat4(1.0f), glm::vec3(puckMesh.offset.x, puckMesh.offset.y, puckMesh.offset.z));

        //update player's paddle position

        // Paddle control
        /*
           // if( picked == true )
            {
                std::cout << "PICKED" << std::endl;
                glm::vec4 cursor1 = glm::vec4(rayDestination[0], rayDestination[1], rayDestination[2], 1.0f);
                glm::vec4 object1 = glm::vec4(myPhysics.paddle_Player->getCenterOfMassPosition().getX(), myPhysics.paddle_Player->getCenterOfMassPosition().getY(), myPhysics.paddle_Player->getCenterOfMassPosition().getZ(), 1.0f);
                
                glm::vec4 cursorCoord = projection * view * cursor1;
                glm::vec4 objectCoord = projection * view * object1;
                
                glm::vec3 finalCursor = glm::vec3(cursorCoord);
                glm::vec3 finalObject = glm::vec3(objectCoord);
                glm::vec3 forceVector = glm::vec3(finalCursor.x-finalObject.x, finalCursor.y-finalObject.y, finalCursor.z-finalObject.z);
              
                myPhysics.paddle_Player->applyCentralImpulse( btVector3(10, 10, 10) );
                //myPhysics.paddle_Player->translate(100*btVector3(forceVector.x, forceVector.y, forceVector.y));
                myPhysics.paddle_Player->setAngularVelocity(btVector3(0,0,0));
            
            }*/
/*
            else
            {
                myPhysics.paddle_Player->setLinearVelocity(btVector3(0,0,0));
                myPhysics.paddle_Player->setAngularVelocity(btVector3(0,0,0));  
            }
    
  */      
        myPhysics.paddle_Player->getMotionState()->getWorldTransform(translate);
        
        
        paddlePlayerMesh.offset.x = translate.getOrigin().getX();
        paddlePlayerMesh.offset.y = translate.getOrigin().getY();
        paddlePlayerMesh.offset.z = translate.getOrigin().getZ();
        paddlePlayerMesh.model = glm::translate(glm::mat4(1.0f), glm::vec3(paddlePlayerMesh.offset.x, paddlePlayerMesh.offset.y, paddlePlayerMesh.offset.z));

        //update ai's paddle position
        myPhysics.paddle_AI->getMotionState()->getWorldTransform(translate);
        paddleAIMesh.offset.x = translate.getOrigin().getX();
        paddleAIMesh.offset.y = translate.getOrigin().getY();
        paddleAIMesh.offset.z = translate.getOrigin().getZ();
        paddleAIMesh.model = glm::translate(glm::mat4(1.0f), glm::vec3(paddleAIMesh.offset.x, paddleAIMesh.offset.y, paddleAIMesh.offset.z));

        glutPostRedisplay();//call the display callback
    }
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
    keyStates[key] = true;
}

void keyboardUp(unsigned char key, int x_pos, int y_pos)
{
    keyStates[key] = false;
}


void mouse(int button, int state, int x, int y) 
{
    MOUSE_X = x;
    MOUSE_Y = y;
    
    // detect button release
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP && rayTraceResult == false && picked == true )
    {
        picked = false;
    }
}

void mouseOnTheMove(int x, int y) 
{
    if(!pauseGame) 
    {
    
        float temp_z = (tableMesh.currentPos.first.z + tableMesh.currentPos.second.z)/2.0f;
        float board_halfs_z = abs(tableMesh.currentPos.second.z - temp_z);

            if(MOUSE_X > x) {
                if(DELTA_X_CHANGE > tableMesh.currentPos.first.x + 3.2f)
                    DELTA_X_CHANGE -= abs(float(MOUSE_X - x))/9.0f;
            }
           else {
                if(DELTA_X_CHANGE < tableMesh.currentPos.second.x - 3.2f)
                    DELTA_X_CHANGE += abs(float(MOUSE_X - x))/9.0f;
           }

           if(MOUSE_Y > y) {
               if(DELTA_Y_CHANGE < tableMesh.currentPos.second.z - board_halfs_z)
                    DELTA_Y_CHANGE += abs(float(MOUSE_Y - y))/9.0f;
           }
           else {
               if(DELTA_Y_CHANGE > tableMesh.currentPos.first.z + 3.2f)
                     DELTA_Y_CHANGE -= abs(float(MOUSE_Y - y))/9.0f;
           }

        MOUSE_Y = y;
        MOUSE_X = x;
        
        
        /*
        float normx = (2.0f * x) / w - 1.0f;
        float normy = -((2.0f * y) / h - 1.0f);
        float normz = 1.0f;

        glm::vec4 norm = glm::vec4(normx, normy, normz, 1.0f);
        glm::vec4 normObjectDepth = glm::vec4(normx, normy, normz, 1.0f); // finding cursor project ray to where object is

        norm = glm::inverse(projection)*norm;
        
        glm::mat4 invertedView = glm::inverse(view);
        
        norm *= 100.0f; // projection depth
        normObjectDepth *= myPhysics.paddle_Player->getCenterOfMassPosition().getZ();// depth of paddle
        
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

        rayTraceResult = myPhysics.rayHit( camera_pos, ray_world, hitPointWorld );
        
        if ( rayTraceResult == true)
        {
            picked = true;
        }
        glutPostRedisplay();    
        
        */
    }
}

void special(int key, int x_pos, int y_pos) 
{
	specialKeyStates[key] = true;
}

void specialUp(int key, int x_pos, int y_pos) 
{
	specialKeyStates[key] = false;
}

void keyOperations(float dt) 
{
    // Handle keyboard input
    if(keyStates[27]) 
    {
        exit(0);
    }
    if(keyStates['p'])
    {
        pauseTheGame();
    }
    if(keyStates['r']) 
    {
        resetGame();
    }
    if(keyStates['+'] || keyStates['=']) 
    {
        if(sensitivity < 50)
            sensitivity++;
    }
    if(keyStates['_'] || keyStates['-']) 
    {
        if(sensitivity > 1)
            sensitivity--;
    }
    
    if(!pauseGame) 
    {
        float temp_z = (tableMesh.currentPos.first.z + tableMesh.currentPos.second.z)/2.0f;
        float board_halfs_z = abs(tableMesh.currentPos.second.z - temp_z);
        if(ai_control == 1) {
            if(keyStates['d']) 
            { 
                if(DELTA_X_CHANGE_ai < tableMesh.currentPos.second.x - 3.2f)
                    DELTA_X_CHANGE_ai += dt*10*sensitivity;
            }
            if(keyStates['a']) 
            { 
                if(DELTA_X_CHANGE_ai > tableMesh.currentPos.first.x + 3.2f)
                    DELTA_X_CHANGE_ai -= dt*10*sensitivity;
            }
            if(keyStates['w']) 
            {
                //if(DELTA_Y_CHANGE_ai > tableMesh.currentPos.second.z - board_halfs_z)
                if(DELTA_Y_CHANGE_ai < tableMesh.currentPos.second.z - 3.2f)
                    DELTA_Y_CHANGE_ai += dt*10*sensitivity;
            }
            if(keyStates['s']) 
            { 
                //if(DELTA_Y_CHANGE_ai < tableMesh.currentPos.second.z - 3.2f)
                if(DELTA_Y_CHANGE_ai > tableMesh.currentPos.second.z - board_halfs_z)
                    DELTA_Y_CHANGE_ai -= dt*10*sensitivity;
            }
        }
        if(keyStates['q']){
        	if (ai_control == 0){
        		ai_control = 1;
            }
            else
            {
        		ai_control = 0;
        	}
        }

        if(ai_control == 0) {
            if(keyStates['j'])
            {
            	if(ai_paddle_speed < 0.35)
            		ai_paddle_speed += 0.02;
            }
            if(keyStates['k'])
            {
            	if (ai_paddle_speed >0.05)
            		ai_paddle_speed -= 0.02;
            }
       }


        //move the camera around the table based on arrow keys
	    if(specialKeyStates[GLUT_KEY_DOWN]) 
	    {
            theta += dt;
	    }
	    if(specialKeyStates[GLUT_KEY_UP]) 
	    {
            theta -= dt;
	    }
	    if(specialKeyStates[GLUT_KEY_RIGHT]) 
	    {
            phi += dt;
	    }
	    if(specialKeyStates[GLUT_KEY_LEFT]) 
	    {
            phi -= dt;
	    }
        //change the radius of the camera sphere
	    if(specialKeyStates[GLUT_KEY_PAGE_UP]) 
	    {  
	        radius -= dt*25;
	    }
        if(specialKeyStates[GLUT_KEY_PAGE_DOWN]) 
        {   
        	radius += dt*25;
	    }
	    //reset the camera to default view
	    if(specialKeyStates[GLUT_KEY_HOME]) 
	    {
	        defaultCamera();
	    }
	}
}



void updateCamera(float dt) 
{
    // Restrict the angles within 0~360 deg (optional)
    if(theta > 360)
    theta = fmod((double)theta,360.0);
    if(phi > 360)
    phi = fmod((double)phi,360.0);

    // Spherical to Cartesian conversion.   
    eyeX = radius * sin(theta*M_PI) * sin(phi*M_PI);
    eyeY = radius * cos(theta*M_PI);
    eyeZ = radius * sin(theta*M_PI) * cos(phi*M_PI);

    // Reduce theta slightly to obtain another point on the same longitude line on the sphere.
    float tempX = radius * sin(theta*M_PI-dt) * sin(phi*M_PI);
    float tempY = radius * cos(theta*M_PI-dt);
    float tempZ = radius * sin(theta*M_PI-dt) * cos(phi*M_PI);

    // Connect these two points to obtain the camera's up vector.
    upX = tempX - eyeX;
    upY = tempY - eyeY;
    upZ = tempZ - eyeZ;
}

void defaultCamera() 
{
    upX = 0.0, upY = 1.0, upZ = 0.0;
    eyeX = 0.0, eyeY = 50.0, eyeZ = -25.0;
    theta = 0.0, phi = 45.0, radius = 55.9;
    
    view = glm::lookAt( glm::vec3(eyeX, eyeY, eyeZ),//Eye Position
                        glm::vec3(0.0, 0.0, 0.0),   //Focus point
                        glm::vec3(upX, upY, upZ));  // up direction
}



bool initialize()
{
    // Initialize basic geometry and shaders for this example
    string paddleObj    = "../objectFiles/paddle.obj";
    string tableObj     = "../objectFiles/table.obj";
    string wallObj      = "../objectFiles/wall.obj";
    string puckObj      = "../objectFiles/puck.obj";
    string groundObj    = "../objectFiles/ground.obj";

    //load table
    if(!meshManager.loadModel(tableObj, "table")) 
    {
        cout <<"[ERROR] " << tableObj  << " Model could not be loaded" << endl;
        return false;
    }
    
    //load ground
    if(!meshManager.loadModel(groundObj, "ground")) 
    {
        cout << "[ERROR] " << groundObj << " Model could not be loaded" << endl;
        return false;
    }
    
    //load wall
    if(!meshManager.loadModel(wallObj, "walls")) 
    {
        cout << "[ERROR] " << wallObj << " Model could not be loaded" << endl;
        return false;
    }
    
    //load puck
    if(!meshManager.loadModel(paddleObj, "paddle")) 
    {
        cout << "[ERROR] " << paddleObj << " Model could not be loaded" << endl;
        return false;
    }

    //load paddle
    if(!meshManager.loadModel(puckObj, "puck")) 
    {
        cout << "[ERROR] " << puckObj << " Model could not be loaded" << endl;
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

    //--Init the view and projection matrices
    defaultCamera();
    projection = glm::perspective( 90.0f, float(w)/float(h), 0.01f, 100.0f);

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_SMOOTH);
    glDepthFunc(GL_LESS);

    return true;
}


void inGoal() {
    if(AI_SCORE < WIN_SCORE && PLAYER_SCORE < WIN_SCORE) 
    {
        bool reset = false;
        bool player_goal = false;
        if(puckMesh.currentPos.first.z > tableMesh.currentPos.second.z + 5) 
        {
            PLAYER_SCORE++;
            reset = true;
            player_goal = true;
        }
        else if(puckMesh.currentPos.second.z < tableMesh.currentPos.first.z - 5) 
        {
            AI_SCORE++;
            reset = true;
        }
        else if(puckMesh.currentPos.first.x < tableMesh.currentPos.first.x - 4.0 &&
                puckMesh.currentPos.second.x > tableMesh.currentPos.second.x + 4.0) 
        {  
            reset = true;
        }
        if(reset) 
        {
            if(AI_SCORE < WIN_SCORE && PLAYER_SCORE < WIN_SCORE) {
            
                resetPuck();
                if(player_goal)
                    puckMesh.offset.z = -15.0;
                else
                    puckMesh.offset.z = 15.0;
             }
            cout << "Player Score : " << PLAYER_SCORE << endl;
            cout << "AI Score : " << AI_SCORE << endl;
        }
    }
}

void resetPuck() 
{
    puckMesh.initializeMesh("puck");
    puckMesh.initialPos = meshManager.getBounds("puck");
    puckMesh.currentPos = puckMesh.initialPos;
    puckMesh.offset.y = tableMesh.currentPos.first.y+0.6;
    puckMesh.offset.z = -5.0;
    myPhysics.resetPuck(puckMesh);
}

void resetGame() 
{
     AI_SCORE = 0;
     PLAYER_SCORE = 0;
     resetPuck();
}

void pauseTheGame() 
{
    if(pauseGame) 
    {
        glutChangeToMenuEntry(5,"Pause Game", 4);
    }
    else 
    {
        glutChangeToMenuEntry(5,"Resume Game", 4);
    }
    pauseGame = !pauseGame;
    keyStates['p'] = false;
    glutPostRedisplay();
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


void updateAI()
{
    glm::vec3 centerPuck;
    centerPuck.x = (puckMesh.currentPos.first.x + puckMesh.currentPos.second.x)/2.0f;
    centerPuck.y = (puckMesh.currentPos.first.y + puckMesh.currentPos.second.y)/2.0f;
    centerPuck.z = (puckMesh.currentPos.first.z + puckMesh.currentPos.second.z)/2.0f;

    glm::vec3 centerAIPaddle;
    centerAIPaddle.x = (paddleAIMesh.currentPos.first.x + paddleAIMesh.currentPos.second.x)/2.0f;
    centerAIPaddle.y = (paddleAIMesh.currentPos.first.y + paddleAIMesh.currentPos.second.y)/2.0f;
    centerAIPaddle.z = (paddleAIMesh.currentPos.first.z + paddleAIMesh.currentPos.second.z)/2.0f;

    float distance = std::sqrt((centerPuck.x-centerAIPaddle.x)* (centerPuck.x-centerAIPaddle.x)
    		+(centerPuck.y-centerAIPaddle.y)*(centerPuck.y-centerAIPaddle.y)
    		+(centerPuck.z-centerAIPaddle.z)*(centerPuck.z-centerAIPaddle.z));

    if(centerPuck.z > 0 && centerPuck.z < 35 && distance > 4 )
    {
        //move horizontal
        if(paddleAIMesh.offset.x < puckMesh.offset.x) {
            //if(DELTA_X_CHANGE_ai > board_mesh.currentPos.first.x + 3.2f)
               DELTA_X_CHANGE_ai -= ai_paddle_speed;
        }
        else if(paddleAIMesh.offset.x > puckMesh.offset.x) 
        {
            //if(DELTA_X_CHANGE_ai < board_mesh.currentPos.second.x - 3.2f)
               DELTA_X_CHANGE_ai += ai_paddle_speed;
        }
        //move vertical
		if(paddleAIMesh.offset.z < puckMesh.offset.z) 
		{
			//if(DELTA_Y_CHANGE_ai > board_mesh.currentPos.first.z + 3.2f)
			   DELTA_Y_CHANGE_ai += ai_paddle_speed;
		}
		else if(paddleAIMesh.offset.z > puckMesh.offset.z) 
		{
			//if(DELTA_Y_CHANGE_ai < board_mesh.currentPos.second.z - 3.2f)
			   DELTA_Y_CHANGE_ai -= ai_paddle_speed;
		}
    }
    else {
    	if(paddleAIMesh.offset.x < 0)
    	{
    		DELTA_X_CHANGE_ai -= ai_paddle_speed;
    	}else{
    		DELTA_X_CHANGE_ai += ai_paddle_speed;
    	}

		if (paddleAIMesh.offset.z < 28) {
			DELTA_Y_CHANGE_ai += ai_paddle_speed;
		} else {
			DELTA_Y_CHANGE_ai -= ai_paddle_speed;
		}
    }
}

void createMenus() 
{

    int _themes_menu = glutCreateMenu(themesMenu);
    glutAddMenuEntry("DISNEY", 1);
    glutAddMenuEntry("MATRIX", 2);
    glutAddMenuEntry("TECHNO", 3);
    glutAddMenuEntry("WOOD", 4);

    int _ai_level_menu = glutCreateMenu(aiLevelMenu);
    glutAddMenuEntry("Level 1", 1);
    glutAddMenuEntry("Level 2", 2);
    glutAddMenuEntry("Level 3", 3);
    glutAddMenuEntry("Level 4", 4);
    glutAddMenuEntry("Level 5", 5);

    glutCreateMenu(mainMenu);
    glutAddSubMenu("THEMES", _themes_menu);
    glutAddSubMenu("AI Level", _ai_level_menu);


    glutAddMenuEntry("AI->Human", 2);
    glutAddMenuEntry("Pause Game", 4);
    glutAddMenuEntry("Restart", 3);
    glutAddMenuEntry("QUIT", 1);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void themesMenu(int id) {
    switch(id) {
      case 1:
        THEME = 0;
        break;
      case 2:
        THEME = 1;
        break;
      case 3:
        THEME = 2;
        break;
      case 4:
        THEME= 3;
        break;
    }
}


void aiLevelMenu(int id) {
    switch(id) {
      case 1:
    	  ai_paddle_speed = 0.1;
        break;
      case 2:
    	  ai_paddle_speed = 0.20;
        break;
      case 3:
    	  ai_paddle_speed = 0.25;
        break;
      case 4:
    	  ai_paddle_speed = 0.30;
        break;
      case 5:
    	  ai_paddle_speed = 0.40;
        break;
    }
}

void mainMenu(int id) {
    switch(id) {
        case 1:
            exit(0);
            break;
        case 2:
            if (ai_control == 0) {
	            ai_control = 1;
	            glutChangeToMenuEntry(4,"Human->AI", 2);
	            }
        	else {
            	ai_control = 0;
            	glutChangeToMenuEntry(4,"AI->Human", 2);
        	}
            break;
        case 3:
            resetGame();
            break;
        case 4:
            pauseTheGame();
            break;
   }
    glutPostRedisplay();
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


