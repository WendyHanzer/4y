#include <iostream>
#include <chrono>

#include<stdio.h>
#include<stdlib.h>
#include "shaderLoader.h"
#include "modelLoader.h"

const GLint PLANETS = 2;
//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

bool spin = true;
int spinDirection = 1;
int transDirection = 1;

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model[PLANETS];//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection
glm::mat4 mvp2;
glm::mat4 rotate;
glm::mat4 translate;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void menu( int selection);
void keyboard(unsigned char key, int x_pos, int y_pos);
void specialKeyboard( int key, int x, int y);
void mouse(int button, int state, int x, int y);

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
{
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);

    // Name and create the Window
    glutCreateWindow("Matrix Example");

    // Create the menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Pause Rotation", 1);
    glutAddMenuEntry("Continue Rotation", 2);
    glutAddMenuEntry("Change Rotation spinDirection", 3);
    glutAddMenuEntry("Quit Program", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

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
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouse);//Called if there is a mouse input

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
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
    for(int i = 0; i < PLANETS; i++)
    {
        mvp = projection * view * model[i];

        //enable the shader program
        glUseProgram(program);

        //upload the matrix to the shader
        glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

        //set up the Vertex Buffer Object so it can be drawn
        glEnableVertexAttribArray(loc_position);
        glEnableVertexAttribArray(loc_color);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);

        //set pointers into the vbo for each of the attributes(position and color)
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer( loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));

        glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count
    }

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();

}

void update()
{
    //total time
    static float transAngle = 0.0;
    static float moonAngle = 0.0;
    static float spinAngle = 0.0;  
    static float moonRotAngle = 0.0;  
    float dt = getDT();// if you have anything moving, use dt.
    
    //planet movement
    if(transDirection == 1)
    {
        transAngle += dt * M_PI/2;
    }
    else
    {
        transAngle -= dt * M_PI/2;
    }

    model[0] = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(transAngle), 1.0, 4.0 * cos(transAngle)));
    
    //check to see if spin is enabled
    if(spin)
    {
        //check for pause
        if(spinDirection == 1)
        {
            spinAngle += dt * 180/M_PI;
        }
        else
        {
            spinAngle -= dt * 180/M_PI;
        }
    }
    
    model[0] = glm::rotate(model[0], spinAngle, glm::vec3(0.0f,1.0f,0.0f));

    //moon movement
    moonAngle += dt * M_PI/2; //move through 90 degrees a second
    moonRotAngle += dt * 120/M_PI;

    model[1] = glm::scale( model[0], glm::vec3(0.25f));
    model[1] = glm::translate( model[1], glm::vec3(8.0 * sin(moonAngle), 1.0, 8.0 * cos(moonAngle)));
    model[1] = glm::rotate(model[1], moonRotAngle, glm::vec3(0.0f,1.0f,0.0f));

    // Update the state of the scene
    glutPostRedisplay();//call the display callback

}


void menu(int selection)
{
    switch(selection)
    {
        case 1:
            spin = false;
            break;

        case 2:
            spin = true;
            break;

        case 3:
            spinDirection *= -1;
            break;

        case 4:
            exit(0);
            break;
    }

    glutPostRedisplay();
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
    switch(key)
    {
        case 27: //escape
            exit(0);
            break;

        case 112: //p
            spin = !spin;
            break;

        case 32: //space
            spinDirection *= -1;
            break;
    }
            
}

void specialKeyboard(int key, int x, int y)
{
    // Special GLUT keys
    switch(key)
    {
        case GLUT_KEY_LEFT: //left arrow key
            transDirection = -1;
            break;

        case GLUT_KEY_RIGHT: //right arrow key
            transDirection = 1;
            break;
    }
}
void mouse(int button, int state, int x, int y)
{
    //Handle mouse input
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        spinDirection *= -1;
    }

}

bool initialize()
{
    // Initialize basic geometry and shaders for this example

    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    char OBJFile[] = "../src/table.obj";
    Vertex geometry[] = loadOBJ(OBJFile);
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    char vertexFile[] = "../src/shaders/default.vert";
    char fragmentFile[] = "../src/shaders/default.frag";

    const char *vs = readFile(vertexFile);
    const char *fs = readFile(fragmentFile);


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
