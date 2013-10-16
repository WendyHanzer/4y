#include <iostream>
#include "texture.h"

Texture::Texture(GLenum TextureTarget, const std::string& FileName)
{

    myfileName = FileName;
    mypImage = NULL;
    mytextureTarget = TextureTarget;

}

bool Texture::Load()
{
    //load texture from file using IM
    mypImage = new Magick::Image(myfileName);
    mypImage->write(&myblob, "RGBA");


    glGenTextures(1, &mytextureObj); //generate texture objects
    glBindTexture(mytextureTarget, mytextureObj);
    glTexImage2D(mytextureTarget, 0, GL_RGB, mypImage->columns(), mypImage->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, myblob.data());
    glTexParameterf(mytextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(mytextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(mytextureTarget, mytextureObj);
}
