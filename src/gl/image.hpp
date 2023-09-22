#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "texture.hpp"
#include "../core/http.hpp"

enum ImageEncoding {
  IMAGE_ENCONDING_SRGB,
  IMAGE_ENCONDING_LINEAR,
};

class Image: public Texture {
  public:
    Image(const ImageEncoding encoding, HTTPRequest* request);
    ~Image();
    GLuint get();
    const glm::vec2& getSize();
  private:
    const ImageEncoding encoding;
    HTTPRequest* request;
    glm::vec2 size;
    void update();
};
