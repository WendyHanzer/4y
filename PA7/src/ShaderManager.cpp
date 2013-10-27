#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include <iostream>
#include <fstream>

#include "ShaderManager.h"
#include "shaderLoader.h"


ShaderManager::ShaderManager()
{ 
    program = glCreateProgram();  
}


ShaderManager::~ShaderManager() 
{ 
}


bool ShaderManager::loadVertexShader(char *fileName) 
{ 

    //create shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    std::ifstream fin;
    fin.open(fileName);
    char *vbuffer;

    int charcount;

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
    
    const char *vsName = vbuffer;

    //set shader source then compile
    glShaderSource(vertexShader, 1, &vsName, NULL);

    glCompileShader(vertexShader);
    
    //check compile status
    GLint shaderStatus;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderStatus); 
    if(!shaderStatus)
    {
      std::cout << "Vertex shader failed to compile - please ensure file name is correct and try again.";
      return false;
    }

    //attach to program 
    glAttachShader(program, vertexShader);
     
    return true;
}


bool ShaderManager::loadFragmentShader(char *fileName) 
{
    //create shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::ifstream fin;
    fin.open(fileName);
    char *fbuffer;

    int charcount;

    if ( fin.is_open() )
    {
        fin.seekg(0, std::ios::end);
        charcount = fin.tellg();
        fbuffer = new char[ charcount + 1 ];
        fin.seekg(0, std::ios::beg);
        fin.read( fbuffer, charcount );
        fbuffer[ charcount ] = '\0';
        fin.close();
    }
    
    const char *fsName = fbuffer;
   
    //set shader source then compile
    glShaderSource(fragmentShader, 1, &fsName, NULL);
    glCompileShader(fragmentShader);

    //check compile status
    GLint shaderStatus;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderStatus);  
    if(!shaderStatus) 
    {
     std::cout << "Fragment shader failed to compile - please ensure file name is correct and try again.";	
     return false;
    }

    //attach to program
    glAttachShader(program, fragmentShader);

    return true;
}    


void ShaderManager::linkProgramObject() 
{
    glLinkProgram(program);
}


void ShaderManager::useProgram() 
{
    glUseProgram(program);
}


GLint ShaderManager::getAttributeLocation(char *attribName)
{
    return glGetAttribLocation( program, attribName );
}

GLint ShaderManager::getUniformAttributeLocation(char *attribName)
{
    return glGetUniformLocation( program, attribName );
}





















