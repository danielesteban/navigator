#include "mesh.hpp"
#include <glm/gtc/matrix_inverse.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Mesh::Mesh(Geometry* geometry, Shader* shader):
  Object(),
  body(nullptr),
  bounds({ glm::vec3(0.0, 0.0, 0.0 ), 0.0 }),
  boundsVersion(0),
  frustumCulling(true),
  geometry(geometry),
  needsBoundsUpdate(false),
  needsTransformUpdate(false),
  normalTransform(glm::mat3(1.0)),
  position(glm::vec3(0.0, 0.0, 0.0)),
  rotation(glm::quat(1.0, 0.0, 0.0, 0.0)),
  scale(glm::vec3(1.0, 1.0, 1.0)),
  shader(shader),
  transform(glm::mat4(1.0))
{
  
}

Mesh::~Mesh() {
  for (const auto& [name, texture]: uniformsTexture) {
    Texture::gc(texture);
  }
}

btRigidBody* Mesh::getBody() {
  return body;
}

void Mesh::setBody(btRigidBody* value) {
  body = value;
}

const GeometryBounds& Mesh::getBounds() {
  if (needsTransformUpdate) {
    updateTransform();
  }
  if (needsBoundsUpdate || geometry->needsUpdate || boundsVersion != geometry->getVersion()) {
    needsBoundsUpdate = false;
    bounds = geometry->getBounds(transform);
    boundsVersion = geometry->getVersion();
  }
  return bounds;
}

bool Mesh::getFrustumCulling() {
  return frustumCulling;
}

void Mesh::setFrustumCulling(const bool enabled) {
  frustumCulling = enabled;
}

Geometry* Mesh::getGeometry() {
  return geometry;
}

Shader* Mesh::getShader() {
  return shader;
}

const glm::vec3& Mesh::getPosition() {
  return position;
}

void Mesh::setPosition(const glm::vec3& value) {
  position = value;
  needsTransformUpdate = true;
}

const glm::quat& Mesh::getRotation() {
  return rotation;
}

void Mesh::setRotation(const glm::quat& value) {
  rotation = value;
  needsTransformUpdate = true;
}

const glm::vec3& Mesh::getScale() {
  return scale;
}

void Mesh::setScale(const glm::vec3& value) {
  scale = value;
  needsTransformUpdate = true;
}

void Mesh::lookAt(const glm::vec3 & target) {
  glm::vec3 direction = glm::normalize(position - target);
  glm::vec3 up = glm::vec3(0, 1, 0);
  if (glm::abs(glm::dot(direction, up)) > 0.9999) {
    up = glm::vec3(0, 0, 1);
  }
  rotation = glm::quatLookAt(direction, up);
  needsTransformUpdate = true;
}

const glm::mat4& Mesh::getTransform() {
  if (needsTransformUpdate) {
    updateTransform();
  }
  return transform;
}

void Mesh::setUniformInt(const std::string& name, const GLint value) {
  uniformsInt[name] = value;
}

void Mesh::setUniformFloat(const std::string& name, const GLfloat value) {
  uniformsFloat[name] = value;
}

void Mesh::setUniformTexture(const std::string& name, Texture* texture) {
  if (uniformsTexture.contains(name)) {
    Texture::gc(uniformsTexture[name]);
  }
  texture->refs++;
  uniformsTexture[name] = texture;
}

void Mesh::setUniformVec2(const std::string& name, const glm::vec2& value) {
  uniformsVec2[name] = value;
}

void Mesh::setUniformVec3(const std::string& name, const glm::vec3& value) {
  uniformsVec3[name] = value;
}

void Mesh::setUniformVec4(const std::string& name, const glm::vec4& value) {
  uniformsVec4[name] = value;
}

void Mesh::updateTransform() {
  needsTransformUpdate = false;
  needsBoundsUpdate = true;
  transform = glm::translate(glm::mat4(1.0), position);
  transform *= glm::toMat4(rotation);
  transform = glm::scale(transform, scale);
  normalTransform = glm::inverseTranspose(glm::mat3(transform));
}

void Mesh::render(Camera* camera, const GLsizei instances) {
  if (
    shader == nullptr
    || (
      frustumCulling && !camera->isInFrustum(getBounds())
    )
  ) {
    return;
  }
  if (needsTransformUpdate) {
    updateTransform();
  }
  shader->setCameraUniforms(camera);
  shader->setTextureUniforms(&uniformsTexture);
  shader->setUniformMat4("modelMatrix", transform);
  shader->setUniformMat3("normalMatrix", normalTransform);
  for (const auto& [name, value]: uniformsInt) {
    shader->setUniformInt(name.c_str(), value);
  }
  for (const auto& [name, value]: uniformsFloat) {
    shader->setUniformFloat(name.c_str(), value);
  }
  for (const auto& [name, value]: uniformsVec2) {
    shader->setUniformVec2(name.c_str(), value);
  }
  for (const auto& [name, value]: uniformsVec3) {
    shader->setUniformVec3(name.c_str(), value);
  }
  for (const auto& [name, value]: uniformsVec4) {
    shader->setUniformVec4(name.c_str(), value);
  }
  shader->use();
  geometry->draw(instances);
}
