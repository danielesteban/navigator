#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "texture.hpp"

class Framebuffer {
  public:
    Framebuffer(const GLint numTextures, const bool depth, const GLint samples);
    ~Framebuffer();
    GLint width, height;
    const bool depth;
    bool isBinded();
    void bind();
    Texture* getTexture(const GLint index);
    void blit(Framebuffer* target, GLint targetWidth, GLint targetHeight);
    void clear();
    bool setSize(GLint width, GLint height);
    bool setTextureClearColor(const GLint index, const glm::vec4& color);
    bool setTextureData(const GLint index, const GLfloat* data);
  private:
    GLuint fbo;
    GLuint rbo;
    std::vector<glm::vec4> clearColors;
    std::vector<Texture*> textures;
    const bool multisampled;
    const GLuint samples;
};
