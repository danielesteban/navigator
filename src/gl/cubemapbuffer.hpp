#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "primitives/box.hpp"
#include "shader.hpp"

class Cubemapbuffer {
  public:
    Cubemapbuffer();
    ~Cubemapbuffer();
    void renderHDR(GLfloat* data, GLint width, GLint height, GLuint& output, GLint outputWidth, GLint outputHeight);
    void renderIrradiance(GLuint input, GLuint& output, GLint outputWidth, GLint outputHeight);
    void renderPrefiltered(GLuint input, GLuint& output, GLint outputWidth, GLint outputHeight);
  private:
    GLuint fbo;
    BoxGeometry box;
    Shader shaderCubemap;
    Shader shaderIrradiance;
    Shader shaderPrefiltered;
    void create(GLuint& texture, const GLint width, const GLint height, const bool trilinear = false);
    void render(const GLuint output, const GLint width, const GLint height, Shader& shader, const GLuint mipLevels = 1);
    void dump(const GLuint texture, const GLint width, const GLint height);
};
