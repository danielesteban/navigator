#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture {
  public:
    const GLuint id;
    GLuint refs;
    Texture(const GLenum binding = GL_TEXTURE_2D);
    virtual ~Texture();
    static void gc(Texture* texture);
    void bind(const GLint target);
    virtual GLuint get();
  protected:
    const GLenum binding;
    GLuint texture;
  private:
    static GLuint textureId;
};
