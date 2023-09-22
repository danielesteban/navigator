#include "texture.hpp"

GLuint Texture::textureId = 1;

Texture::Texture(const GLenum binding):
  id(textureId++),
  refs(1),
  binding(binding),
  texture(0)
{

}

Texture::~Texture() {
  if (texture != 0) {
    glDeleteTextures(1, &texture);
  }
}

void Texture::gc(Texture* texture) {
  texture->refs--;
  if (texture->refs == 0) {
    delete texture;
  }
}

void Texture::bind(const GLint target) {
  glActiveTexture(GL_TEXTURE0 + target);
  glBindTexture(binding, get());
}

GLuint Texture::get() {
  if (texture == 0) {
    glGenTextures(1, &texture);
  }
  return texture;
}
