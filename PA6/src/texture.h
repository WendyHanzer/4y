#include <string>
#include <GL/glew.h>
#include <ImageMagick/Magick++.h>

class Texture
{
public:

    //create texture object
    Texture(GLenum TextureTarget, const std::string& FileName);
    //load texture
    bool Load();
    //bind specific texture to texture unit
    void Bind(GLenum TextureUnit);

private:
    std::string myfileName;
    GLenum mytextureTarget;
    GLuint mytextureObj;
    Magick::Image* mypImage;
    Magick::Blob myblob;
};

