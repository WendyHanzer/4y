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



MeshManager meshManager;
ShaderManager *shaderManager;
GLuint tanTex, green, blue;
GLint textureCoord;
GLint TexUnit;


GLint scene_mv;
GLint scene_p;
GLint scene_position;

//meshes
Mesh table;
Mesh sphere;
Mesh wallOne;
Mesh wallTwo;
Mesh wall_three;
Mesh wall_four;
Mesh cube;
Mesh cylinder;
Mesh tempMesh;

//transform matrices
glm::mat4 view;         //world->eye
glm::mat4 projection;   //eye->clip
glm::mat4 mv;           //model-view matrix


Physics m_Physics;

float offsetx=0.0;
float offsetz=0.0;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);

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
    glutCreateWindow("PA8 - Bullet");

    //initialize textures
    char blueTexture[] = "../textures/blue.jpg";
    char greenTexture[] = "../textures/green.jpg";
    char tanTexture[] = "../textures/tan.jpg";

    //load textures
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
    
    //Initialize all of our resources
    shaderManager = new ShaderManager();

    bool init = initialize(); //initialize shaders and load models  

    //Initialize all of our meshes
    tempMesh.initializeMesh("sphere");
    tempMesh.initial_bound = meshManager.getBounds("sphere");   //set current position
    tempMesh.current_position = tempMesh.initial_bound;         //set current position
    sphere = tempMesh;
    sphere.offset.y = table.current_position.second.y+1.5;
    sphere.offset.z = -5;

    
    tempMesh.initializeMesh("cube");
    tempMesh.initial_bound = meshManager.getBounds("cube");     //set current position
    tempMesh.current_position = tempMesh.initial_bound;         //set current position
    cube = tempMesh;
    cube.offset.y = table.current_position.second.y+10;
    cube.offset.z = 7;


    tempMesh.initializeMesh("cylinder");
    tempMesh.initial_bound = meshManager.getBounds("cylinder"); //set current position
    tempMesh.current_position = tempMesh.initial_bound;         //set current position
    cylinder = tempMesh;
    cylinder.offset.y = table.current_position.second.y+2;
    cylinder.offset.z = 2;

    
    tempMesh.initializeMesh("table");
    tempMesh.initial_bound = meshManager.getBounds("table");
    tempMesh.current_position = tempMesh.initial_bound;
    table = tempMesh;
    
    tempMesh.initializeMesh("wall one");
    tempMesh.initial_bound = meshManager.getBounds("wallOne"); //set current position
    tempMesh.current_position = tempMesh.initial_bound;         //set current position
    wallOne = tempMesh;
    wallOne.offset.y = table.current_position.second.y;        //top of the table
    wallOne.offset.x = 9;
    wallTwo = wallOne;
    wallTwo.offset.x = -9;

    tempMesh.initializeMesh("wall two");
    tempMesh.initial_bound = meshManager.getBounds("wallTwo"); //set current position
    tempMesh.current_position = tempMesh.initial_bound;         //set current position
    wall_three = tempMesh;
    wall_three.offset.y = table.current_position.second.y;      //top of the table
    wall_three.offset.z = 9;
    wall_four = wall_three;
    wall_four.offset.z = -9;



    //update walls 
    wallOne.model=glm::translate(glm::mat4(1.0f), glm::vec3(   wallOne.offset.x, 
                                                                wallOne.offset.y, 
                                                                wallOne.offset.z));

    wallTwo.model=glm::translate(glm::mat4(1.0f), glm::vec3(   wallTwo.offset.x, 
                                                                wallTwo.offset.y, 
                                                                wallTwo.offset.z));

    wall_three.model=glm::translate(glm::mat4(1.0f), glm::vec3( wall_three.offset.x,
                                                                wall_three.offset.y, 
                                                                wall_three.offset.z));

    wall_four.model=glm::translate(glm::mat4(1.0f), glm::vec3(  wall_four.offset.x, 
                                                                wall_four.offset.y, 
                                                                wall_four.offset.z));

    //initialize the physics
    m_Physics.makeSphere(sphere);
    m_Physics.makeCube(cube);
    m_Physics.makeCylinder(cylinder);
    m_Physics.makeTable(table);
    m_Physics.makeWall(wallOne);
    m_Physics.makeWall(wallTwo);
    m_Physics.makeWall(wall_three);
    m_Physics.makeWall(wall_four);

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


 /*
    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

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

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();

*/

    //////////////////
    /* SET UP TABLE */
    //////////////////

    //get mv matrix
    mv = view * table.model;

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
    glBindTexture(GL_TEXTURE_2D, tanTex);
    glUniform1i(TexUnit, 0);

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
    /* SET UP SPHERE */
    ///////////////////
    
    //get model view matrix
    mv = view * table.model * sphere.model;

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
    glBindTexture(GL_TEXTURE_2D, blue);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("sphere"));

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
                           
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("sphere"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);
    
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////
    /* SET UP CUBE */
    /////////////////
    
    //get model view matrix
    mv = view * table.model * cube.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blue);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("static_cube"));

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
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("static_cube"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////
    /* SET UP CYLINDER */
    /////////////////////
    
    //get model view matrix
    mv = view*table.model*cylinder.model;

    //enable the shader program
    shaderManager->useProgram();

    //upload the matrix to the shader -- all the uniforms
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glUniformMatrix4fv(scene_p, 1, GL_FALSE, glm::value_ptr(projection));  //projection

    //upload the lights
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(scene_position);
    glEnableVertexAttribArray(textureCoord);

    //set textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blue);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("cylinder"));

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
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("cylinder"));//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    //////////////////
    /* SET UP WALLS */
    //////////////////
    
    //WALL ONE -------------------------------------------------------
    
    //get model view matrix
    mv = view * table.model * wallOne.model;

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
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("wallOne"));

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
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wallOne"));//mode, starting index, count


    //WALL TWO -------------------------------------------------------
    
    //get model view matrix
    mv = view * table.model  * wallTwo.model; 
    
    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wallOne")); //mode, starting index, count

    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);


    //WALL THREE -------------------------------------------------------
    
    //get model view matrix
    mv = view*table.model*wall_three.model; //model * view

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
    glBindTexture(GL_TEXTURE_2D, green);
    glUniform1i(TexUnit, 0);

    glBindBuffer(GL_ARRAY_BUFFER, meshManager.getHandle("wallTwo"));

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
                            
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wallTwo"));//mode, starting index, count

    //WALL FOUR -------------------------------------------------------
    mv = view * table.model  *wall_four.model; //model * view
    
    //upload the matrix to the shader
    glUniformMatrix4fv(scene_mv, 1, GL_FALSE, glm::value_ptr(mv));         //model view
    glDrawArrays(GL_TRIANGLES, 0, meshManager.getNumVertices("wallTwo")); //mode, starting index, count
   
    //clean up
    glDisableVertexAttribArray(scene_position);
    glDisableVertexAttribArray(textureCoord);
    glDisable(GL_TEXTURE_2D);


    //swap the buffers
    glutSwapBuffers();
}

void update() {

    //total time
    float dt = getDT(); // if you have anything moving, use dt.
    btTransform trans;

    //simulate
    m_Physics.simulate();

    ///////////////////
    /* UPDATE SPHERE */
    ///////////////////
    
    m_Physics.simulationSphere->getMotionState()->getWorldTransform(trans);

    //update the x,y, and z
    sphere.offset.x = trans.getOrigin().getX();
    sphere.offset.y = trans.getOrigin().getY();
    sphere.offset.z = trans.getOrigin().getZ();

    btQuaternion qt = trans.getRotation();
    btVector3 axes = qt.getAxis();
    float radAngle = float((qt.getAngle())*180/PI);

    sphere.model=glm::translate(glm::mat4(1.0f), glm::vec3(sphere.offset.x, sphere.offset.y, sphere.offset.z));
    sphere.model=glm::rotate(sphere.model,radAngle, glm::vec3(axes.getX(),axes.getY(),axes.getZ() ));
    

    /////////////////////
    /* UPDATE CYLINDER */
    /////////////////////

    m_Physics.moveCylinder( cylinder, offsetx, offsetz);
    offsetx = 0.0;
    offsetz = 0.0;

    m_Physics.simulationCylinder->getMotionState()->getWorldTransform(trans);


    axes = qt.getAxis();


    cylinder.model=glm::translate(glm::mat4(1.0f), glm::vec3(   cylinder.offset.x,
                                                                cylinder.offset.y,
                                                                cylinder.offset.z));


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
            
        case 97: //a
            offsetx += .25;
            break;
            
        case 100: //d
            offsetx -= .25;
            break;
            
        case 119: //s
            offsetz += .25;
            break;
            
        case 115: //w
            offsetz -= .25;
            break;    
    }
}


bool initialize()
{
    // Initialize basic geometry and shaders for this example
    string sphereObj  = "../objectFiles/sphere.obj";
    string tableObj   = "../objectFiles/table.obj";
    string wallOneObj = "../objectFiles/wallOne.obj";
    string wallTwoObj = "../objectFiles/wallTwo.obj";
    string cubeObj    = "../objectFiles/cube.obj";
    string cylinderObj= "../objectFiles/cylinder.obj";

    //load table
    if(!meshManager.loadModel(tableObj, "table")) 
    {
        cout <<"[ERROR] " << tableObj  << " Model could not be loaded" << endl;
        return false;
    }
    
    //load walls
    if(!meshManager.loadModel(wallOneObj, "wallOne")) 
    {
        cout << "[ERROR] " << wallOneObj << " Model could not be loaded" << endl;
        return false;
    }
    if(!meshManager.loadModel(wallTwoObj, "wallTwo")) 
    {
        cout << "[ERROR] " << wallTwoObj << " Model could not be loaded" << endl;
        return false;
    }
    
    //load ball
    if(!meshManager.loadModel(sphereObj, "sphere")) 
    {
        cout << "[ERROR] " << sphereObj << " Model could not be loaded" << endl;
        return false;
    }

    //load cube
    if(!meshManager.loadModel(cubeObj, "static_cube")) 
    {
        cout << "[ERROR] " << cubeObj << " Model could not be loaded" << endl;
        return false;
    }
    
    //load cylinder
    if(!meshManager.loadModel(cylinderObj, "cylinder")) 
    {
        cout << "[ERROR] " << cylinderObj << " Model could not be loaded" << endl;
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

    TexUnit= shaderManager->getUniformAttributeLocation("texUnit");
    if(TexUnit == -1) 
    {
        std::cerr << "[F] TEXUNIT ATTRIBUTE NOT FOUND " << std::endl;
        return false;
    }

    //--Init the view and projection matrices
    view = glm::lookAt( glm::vec3(0.0, 10.0, -8.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0),   //Focus point
                        glm::vec3(0.0, 1.0, 0.0));  //Positive Y is up

    projection = glm::perspective( 90.0f, float(w)/float(h), 0.01f, 100.0f);

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

//returns the time delta
float getDT() 
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}
