#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Object {
  public:
    Object();
    virtual ~Object();
    GLuint getId();
    GLubyte getFlags();
    void setFlags(const GLubyte value);
  protected:
    const GLuint id;
    GLubyte flags;
  private:
    static GLuint objectId;
};
