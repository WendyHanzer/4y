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
#include"ShaderManager.h" //for managing shaders
#include"Physics.h" //physics

//CONSTS used for user interactions
int w = 640, h = 480;// Window size
int MOUSE_X = 0, MOUSE_Y = 0; //used for mouse interface
float DELTA_X_CHANGE = 0.0, DELTA_Y_CHANGE = 0.0; //for mouse and keyboard interaction
double X_CHANGE = 0.05, Y_CHANGE = 0.05; //angle step change in DELTA_X_CHANGE and DELTA_Y_CHANGE from keyboard

//managers
MeshManager meshManager;
ShaderManager *shaderManager;

//shader attribute handles
GLint scene_mv;
GLint scene_p;
GLint scene_position;

//texture handles
GLuint white_marble, black_marble;
GLint TexCoord;
GLint TexUnit;

//transform matrices
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mv;

//meshes -- Mesh objects keep track of the bounding boxes of each object that is rendered
//refer to Mesh.h
Mesh table_mesh;
Mesh sphere_mesh;
Mesh wall_mesh_one;
Mesh wall_mesh_two;
Mesh wall_mesh_three;
Mesh wall_mesh_four;
Mesh static_cube_mesh;
Mesh dynamic_cylinder_mesh;

//physics class instatiation
Physics myPhysics;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);
void mouseOnTheMove(int x, int y);

//--Resource management
bool initialize();
void loadTexture(const char* name, GLuint &textID);

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--Main
int main(int argc, char **argv) {
    //Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    //Name and create the Window
    glutCreateWindow("PHYSICS EXAMPLE PROGRAM");

    //initialize textures --> [NOTE] texture coordinates are retrieved from .obj files by the MeshManager class
    char white_marble_file[] = "../textures/MarbleWhite.png";
    char black_marble_file[] = "../textures/black_marble.jpg";

    //load textures
    Magick::InitializeMagick(*argv);
    loadTexture(white_marble_file, white_marble);
    loadTexture(black_marble_file, black_marble);

    GLenum status = glewInit(); //check glut status
    if( status != GLEW_OK){
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    //Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutIdleFunc(update);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseOnTheMove);

    //Initialize all of our resources
    shaderManager = new ShaderManager();

    bool init = initialize(); //initialize shaders and load models  

    //build mesh objects (bounding boxes)
    Mesh temp_mesh; //temp Mesh model

    temp_mesh.initialize("board");
    temp_mesh.initial_bound = meshManager.getBounds("board");
    temp_mesh.current_position = temp_mesh.initial_bound;
    table_mesh = temp_mesh;

    temp_mesh.initialize("sphere");
    temp_mesh.initial_bound = meshManager.getBounds("sphere"); //set current position
    temp_mesh.current_position = temp_mesh.initial_bound; //set current position
    temp_mesh.offset.y = 7;
    temp_mesh.offset.z = -5;
    sphere_mesh = temp_mesh;

    temp_mesh.initialize("wall one");
    temp_mesh.initial_bound = meshManager.getBounds("wall_one"); //set current position
    temp_mesh.current_position = temp_mesh.initial_bound; //set current position
    wall_mesh_one = temp_mesh;
    wall_mesh_one.offset.y = table_mesh.current_position.second.y; //top of the table
    wall_mesh_one.offset.x = 9;
    wall_mesh_two = wall_mesh_one;
    wall_mesh_two.offset.x = -9;

    temp_mesh.initialize("wall two");
    temp_mesh.initial_bound = meshManager.getBounds("wall_two"); //set current position
    temp_mesh.current_position = temp_mesh.initial_bound; //set current position
    wall_mesh_three = temp_mesh;
    wall_mesh_three.offset.y = table_mesh.current_position.second.y; //top of the table
    wall_mesh_three.offset.z = 9;
    wall_mesh_four = wall_mesh_three;
    wall_mesh_four.offset.z = -9;

    temp_mesh.initialize("static cube");
    temp_mesh.initial_bound = meshManager.getBounds("static_cube"); //set current position
    temp_mesh.current_position = temp_mesh.initial_bound; //set current position
    static_cube_mesh = temp_mesh;
    static_cube_mesh.offset.y = table_mesh.current_position.second.y; //top of the table

    temp_mesh.initialize("cylinder");
    temp_mesh.initial_bound = meshManager.getBounds("cylinder"); //set current position
    temp_mesh.current_position = temp_mesh.initial_bound; //set current position
    dynamic_cylinder_mesh = temp_mesh;
    dynamic_cylinder_mesh.offset.y = table_mesh.current_position.second.y + 3; //top of the table
    dynamic_cylinder_mesh.offset.z = 2;
    dynamic_cylinder_mesh.offset.x = -3;

    //update walls - update Model Matrices of the walls
    wall_mesh_one.model=glm::translate(glm::mat4(1.0f), glm::vec3(wall_mesh_one.offset.x, 
                                                                  wall_mesh_one.offset.y, 
                                                                  wall_mesh_one.offset.z));

    wall_mesh_two.model=glm::translate(glm::mat4(1.0f), glm::vec3(wall_mesh_two.offset.x, 
                                                                  wall_mesh_two.offset.y, 
                                                                  wall_mesh_two.offset.z));

    wall_mesh_three.model=glm::translate(glm::mat4(1.0f), glm::vec3(  wall_mesh_three.offset.x,
                                                                      wall_mesh_three.offset.y, 
                                                                      wall_mesh_three.offset.z));

    wall_mesh_four.model=glm::translate(glm::mat4(1.0f), glm::vec3(   wall_mesh_four.offset.x, 
                                                                      wall_mesh_four.offset.y, 
                                                                      wall_mesh_four.offset.z));


    //initialize the physics - add the objects to the simulated environment
    myPhysics.addBall(sphere_mesh);
    myPhysics.addTable(table_mesh);
    myPhysics.addObstacle(wall_mesh_one);
    myPhysics.addObstacle(wall_mesh_two);
    myPhysics.addObstacle(wall_mesh_three);
    myPhysics.addObstacle(wall_mesh_four);
    myPhysics.addStaticCube(static_cube_mesh);
    myPhysics.addDynamicCyinder(dynamic_cylinder_mesh);

    if(init) {
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

    //draw board--------------------------------------------------------------------------
    mv = view * table_mesh.model;

    //enable the shader program
     shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
     glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
     glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(TexCoord);

    //texture stuff
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, white_marble);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("board"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer( scene_position, 3,GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer( TexCoord, 2 , GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,texCoords) ); //might cause issues offset of
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("board"));//mode, starting index, count

    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(TexCoord);
    glDisable(GL_TEXTURE_2D);
    
    //draw ball-----------------------------------------------------------------------------
    mv = view * table_mesh.model  *sphere_mesh.model; //model * view

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(TexCoord);

    //texture stuff
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black_marble);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("sphere"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer( scene_position, 3,GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer( TexCoord, 2 , GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,texCoords) ); //might cause issues offset of
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("sphere"));//mode, starting index, count

    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(TexCoord);
    glDisable(GL_TEXTURE_2D);

    //draw walls-----------------------------------------------------------------------------
    mv = view * table_mesh.model  *wall_mesh_one.model; //model * view

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(TexCoord);

    //texture stuff
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black_marble);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("wall_one"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer( scene_position, 3,GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer( TexCoord, 2 , GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,texCoords) ); //might cause issues offset of
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wall_one"));//mode, starting index, count

    //wall two
    mv = view * table_mesh.model  *wall_mesh_two.model; //model * view
    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wall_one"));//mode, starting index, count

    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(TexCoord);
    glDisable(GL_TEXTURE_2D);


    //wall three and four
    mv = view*table_mesh.model*wall_mesh_three.model; //model * view

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(TexCoord);

    //texture stuff
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black_marble);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("wall_two"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer( scene_position, 3,GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer( TexCoord, 2 , GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,texCoords) ); //might cause issues offset of
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wall_two"));//mode, starting index, count

    //wall four
    mv = view * table_mesh.model  *wall_mesh_four.model; //model * view
    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wall_two"));//mode, starting index, count
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(TexCoord);
    glDisable(GL_TEXTURE_2D);

    //draw static cube-----------------------------------------------------------------------------
    mv = view*table_mesh.model*static_cube_mesh.model; //model * view

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(TexCoord);

    //texture stuff
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black_marble);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("static_cube"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer( scene_position, 3,GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer( TexCoord, 2 , GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,texCoords) ); //might cause issues offset of
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("static_cube"));//mode, starting index, count

    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(TexCoord);
    glDisable(GL_TEXTURE_2D);

    //draw dynamic cylinder-----------------------------------------------------------------------------
    mv = view*table_mesh.model*dynamic_cylinder_mesh.model; //model * view

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(TexCoord);

    //texture stuff
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, black_marble);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("cylinder"));

    //set pointers into the vbo for each of the attributes(position and normal)
    glVertexAttribPointer( scene_position, 3,GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glVertexAttribPointer( TexCoord, 2 , GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,texCoords) ); //might cause issues offset of
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("cylinder"));//mode, starting index, count

    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(TexCoord);
    glDisable(GL_TEXTURE_2D);

    //swap the buffers
    glutSwapBuffers();
}

void update() {
    //total time
    float dt = getDT();// if you have anything moving, use dt.
    btTransform trans; //"Model Matrix" from Bullet

    //simulate
    myPhysics.simulate(); //simulate world

    //update sphere
    myPhysics.simulationBall->getMotionState()->getWorldTransform(trans);

    //update the x,y, and z
    sphere_mesh.offset.x = trans.getOrigin().getX();
    sphere_mesh.offset.y = trans.getOrigin().getY();
    sphere_mesh.offset.z = trans.getOrigin().getZ();

    btQuaternion qt = trans.getRotation();
    btVector3 axes = qt.getAxis();
    float radAngle = float((qt.getAngle())*180/PI);

    sphere_mesh.model=glm::translate(glm::mat4(1.0f), glm::vec3(sphere_mesh.offset.x, sphere_mesh.offset.y, sphere_mesh.offset.z));
    sphere_mesh.model=glm::rotate(sphere_mesh.model,radAngle, glm::vec3(axes.getX(),axes.getY(),axes.getZ() ));

    //update cylinder
    myPhysics.simulationCylinder->getMotionState()->getWorldTransform(trans);

    //update the x,y, and z
    dynamic_cylinder_mesh.offset.x = trans.getOrigin().getX();
    dynamic_cylinder_mesh.offset.y = trans.getOrigin().getY();
    dynamic_cylinder_mesh.offset.z = trans.getOrigin().getZ();

    qt = trans.getRotation();
    axes = qt.getAxis();
    radAngle = float((qt.getAngle())*180/PI);

    dynamic_cylinder_mesh.model=glm::translate(glm::mat4(1.0f), glm::vec3(dynamic_cylinder_mesh.offset.x,
                                                                          dynamic_cylinder_mesh.offset.y,
                                                                          dynamic_cylinder_mesh.offset.z));
    dynamic_cylinder_mesh.model=glm::rotate(dynamic_cylinder_mesh.model,radAngle, glm::vec3(axes.getX(),axes.getY(),axes.getZ() ));


    //update the static cube's location
    myPhysics.simulationStaticCube->getMotionState()->getWorldTransform(trans);

    //update the x,y, and z
    static_cube_mesh.offset.x = trans.getOrigin().getX();
    static_cube_mesh.offset.y = trans.getOrigin().getY();
    static_cube_mesh.offset.z = trans.getOrigin().getZ();

    static_cube_mesh.model=glm::translate(glm::mat4(1.0f), glm::vec3(static_cube_mesh.offset.x,
                                                                     static_cube_mesh.offset.y,
                                                                     static_cube_mesh.offset.z));

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
    //need keyboard input
}


void mouse(int button, int state, int x, int y) 
{
    //need mouse button
}

void mouseOnTheMove(int x, int y) 
{
    //need mouse movement
}


bool initialize()
{
    // Initialize basic geometry and shaders for this example
    string sphere_obj = "../objectFiles/ball.obj";
    string board_obj = "../objectFiles/table.obj";
    string cube_obj = "../objectFiles/walls_one.obj";
    string cube_obj_2 = "../objectFiles/walls_two.obj";
    string static_cube_obj = "../objectFiles/static_cube.obj";
    string dynamic_cylinder_obj = "../objectFiles/dynamic_cylinder.obj";

    //load board
    if(!meshManager.loadModel(board_obj, "board")) 
    {
        cout <<"[ERROR] " << board_obj  << " Model could not be loaded" << endl;
        return false;
    }
    //load ball
    if(!meshManager.loadModel(sphere_obj, "sphere")) {
        cout << "[ERROR] " << sphere_obj << " Model could not be loaded" << endl;
        return false;
    }
    //load walls
    if(!meshManager.loadModel(cube_obj, "wall_one")) {
        cout << "[ERROR] " << cube_obj << " Model could not be loaded" << endl;
        return false;
    }
    if(!meshManager.loadModel(cube_obj_2, "wall_two")) {
        cout << "[ERROR] " << cube_obj_2 << " Model could not be loaded" << endl;
        return false;
    }
    //load static cube
    if(!meshManager.loadModel(static_cube_obj, "static_cube")) {
        cout << "[ERROR] " << static_cube_obj << " Model could not be loaded" << endl;
        return false;
    }
    //load dynamic cylinder
    if(!meshManager.loadModel(dynamic_cylinder_obj, "cylinder")) {
        cout << "[ERROR] " << dynamic_cylinder_obj << " Model could not be loaded" << endl;
        return false;
    }

    //Shader Sources
    char vertexshader[] = "../src/VertexShader.txt";
    char fragmentshader[] = "../src/FragmentShader.txt";

    //initialize shaders
    if(!shaderManager->loadVertexShader(vertexshader))   
        return false;
    if(!shaderManager->loadFragmentShader(fragmentshader))
        return false;

    //link programs
    shaderManager->linkProgramObject();

    //locate attributes
    scene_mv = shaderManager->getUniformAttributeLocation("ModelView");
    if(scene_mv == -1) {
        std::cerr << "[F] MODELVIEW MATRIX NOT FOUND " << std::endl;
        return false;
    }

    scene_p = shaderManager->getUniformAttributeLocation("Projection");
    if(scene_p == -1) {
        std::cerr << "[F] PROJECTION MATRIX NOT FOUND " << std::endl;
        return false;
    }

    scene_position = shaderManager->getAttributeLocation("v_position");
    if(scene_position == -1) {
        std::cerr << "[F] POSITION ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    TexCoord = shaderManager->getAttributeLocation("v_texCoord");
    if(TexCoord == -1) {
        std::cerr << "[F] V_TEXCOORD ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    TexUnit= shaderManager->getUniformAttributeLocation("texUnit");
    if(TexUnit == -1) {
        std::cerr << "[F] TEXUNIT ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    //--Init the view and projection matrices
    view = glm::lookAt( glm::vec3(0.0, 25.0, -5.0), //Eye Position 8 15
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 90.0f, float(w)/float(h), 0.01f, 100.0f);

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_SMOOTH);
    glDepthFunc(GL_LESS);

    //and its done
    return true;
}

//load an image for texture
void loadTexture(const char* name, GLuint &textID) 
{
    Magick::Image* myImage = new Magick::Image( name );
    Magick::Blob myBlob;
    GLuint textureObj;

    myImage->write(&myBlob, "RGBA");
    
    glGenTextures(1, &textID);
    glBindTexture(GL_TEXTURE_2D, textID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //or GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //or GL_NEAREST
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
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
