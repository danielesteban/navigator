#include "image.hpp"
#include <stb_image.h>

Image::Image(const ImageEncoding encoding, HTTPRequest* request):
  Texture(),
  encoding(encoding),
  request(request),
  size(glm::vec2(0, 0))
{

}

Image::~Image() {
  if (request != nullptr) {
    delete request;
  }
}

GLuint Image::get() {
  if (texture == 0) {
    update();
  }
  return texture;
}

const glm::vec2& Image::getSize() {
  return size;
}

void Image::update() {
  if (request != nullptr && request->isReady) {
    if (request->response.status >= 200 && request->response.status < 400 && request->response.size > 0) {
      GLint x, y, n;
      unsigned char* data = stbi_load_from_memory(request->response.data, request->response.size, &x, &y, &n, 4);
      if (data != nullptr) {
        GLint binding;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
        const GLint format = encoding == IMAGE_ENCONDING_SRGB ? GL_SRGB_ALPHA : GL_RGBA;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, binding);
        stbi_image_free(data);
        size = glm::vec2(x, y);
      }
    }
    delete request;
    request = nullptr;
  }
}
