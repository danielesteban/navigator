#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../texture.hpp"

class BRDF: public Texture {
  public:
    GLuint get();
  private:
    void update();
};
