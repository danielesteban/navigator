#include "framebuffer.hpp"
#include <glm/gtc/type_ptr.hpp>

Framebuffer::Framebuffer(const GLint numTextures, const bool depth, const GLint samples) :
  depth(depth),
  multisampled(samples > 0),
  samples(samples),
  width(0),
  height(0)
{
  glGenFramebuffers(1, &fbo);
  for (GLint i = 0; i < numTextures; i++) {
    clearColors.push_back(glm::vec4(0, 0, 0, 1));
    textures.push_back(new Texture(multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D));
  }
  if (depth) {
    glGenRenderbuffers(1, &rbo);
  }
}

Framebuffer::~Framebuffer() {
  glDeleteFramebuffers(1, &fbo);
  for (const auto& texture : textures) {
    Texture::gc(texture);
  }
  if (depth) {
    glDeleteRenderbuffers(1, &rbo);
  }
}

bool Framebuffer::isBinded() {
  GLint current;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current);
  return current == fbo;
}

void Framebuffer::bind() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

Texture* Framebuffer::getTexture(const GLint index) {
  if (index < 0 || index >= textures.size()) {
    return nullptr;
  }
  return textures.at(index);
}

void Framebuffer::blit(Framebuffer* target, GLint targetWidth, GLint targetHeight) {
  if (target != nullptr) {
    target->bind();
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  std::vector<GLenum> buffers;
  for (GLint i = 0, l = textures.size(); i < l; i++) {
    buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
    glBlitFramebuffer(
      0, 0, width, height,
      0, 0, targetWidth, targetHeight,
      GL_COLOR_BUFFER_BIT, GL_LINEAR
    );
  }
  glDrawBuffers(buffers.size(), buffers.data());
  glReadBuffer(0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void Framebuffer::clear() {
  for (GLint i = 0; const auto& texture : textures) {
    glClearBufferfv(GL_COLOR, i, glm::value_ptr(clearColors.at(i)));
  }
  glClear(GL_DEPTH_BUFFER_BIT);
}

bool Framebuffer::setSize(GLint width, GLint height) {
  if (width == this->width && height == this->height) {
    return true;
  }

  this->width = width;
  this->height = height;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  std::vector<GLenum> buffers;

  if (multisampled) {
    for (GLint i = 0; const auto& texture : textures) {
      buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture->get());
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA32F, width, height, GL_TRUE);
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i++, GL_TEXTURE_2D_MULTISAMPLE, texture->get(), 0);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    if (depth) {
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, width, height);
      glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
  } else {
    for (GLint i = 0; const auto& texture : textures) {
      buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
      glBindTexture(GL_TEXTURE_2D, texture->get());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i++, GL_TEXTURE_2D, texture->get(), 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    if (depth) {
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
      glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
  }

  glDrawBuffers(buffers.size(), buffers.data());

  bool status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  return status;
}

bool Framebuffer::setTextureClearColor(const GLint index, const glm::vec4& color) {
  if (index < 0 || index >= clearColors.size()) {
    return false;
  }
  clearColors.at(index) = color;
  return true;
}

bool Framebuffer::setTextureData(const GLint index, const GLfloat* data) {
  if (multisampled || index < 0 || index >= textures.size()) {
    return false;
  }
  GLint binding;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
  glBindTexture(GL_TEXTURE_2D, textures.at(index)->get());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
  glBindTexture(GL_TEXTURE_2D, binding);
  return true;
}
