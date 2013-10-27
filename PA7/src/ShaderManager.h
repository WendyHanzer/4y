#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string.h>
#include <map>

class ShaderManager 
{
    public: 
        ShaderManager();    //constructor
        ~ShaderManager();   //destructor

        bool loadVertexShader(char *fileName);    //load vertex shader
        bool loadFragmentShader(char *fileName);  //load fragment shader
        void linkProgramObject();                 //link program
        void useProgram();                        //activate program

        GLint getAttributeLocation(char *attribName);
        GLint getUniformAttributeLocation(char *attribName);

    private: 
        GLuint program; //GLSL program handler
};
#endif
