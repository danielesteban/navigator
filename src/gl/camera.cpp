#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const GLfloat Camera::nearPlane = 0.1;
const GLfloat Camera::farPlane = 10000.0;
const glm::vec3 Camera::worldUp = glm::vec3(0.0, 1.0, 0.0);

Camera::Camera(): aspect(0), fov(0), version(1) {
  reset();
}

void Camera::lookAt(const glm::vec3& value) {
  front = glm::normalize(value - position);
  updateView();
  updateFrustum();
}

void Camera::setAspect(const GLfloat value) {
  if (value == aspect) {
    return;
  }
  aspect = value;
  updateProjection();
  updateFrustum();
}

const GLfloat Camera::getFov() {
  return fov;
}

const glm::vec3& Camera::getFront() {
  return front;
}

void Camera::setFov(const GLfloat value) {
  if (value == fov) {
    return;
  }
  fov = value;
  updateProjection();
  updateFrustum();
}

const glm::vec3& Camera::getPosition() {
  return position;
}

void Camera::setPosition(const glm::vec3& value) {
  position = value;
  updateView();
  updateFrustum();
}

const glm::mat4& Camera::getProjection() {
  return projection;
}

const glm::mat4& Camera::getView() {
  return view;
}

const GLuint Camera::getVersion() {
  return version;
}

bool Camera::isInFrustum(const GeometryBounds& bounds) {
  for (int i = 0; i < 6; i++) {
    const GLfloat distance = glm::dot(frustum[i].normal, bounds.position) + frustum[i].constant;
    if (distance < -bounds.radius) {
      return false;
    }
  }
  return true;
}

void Camera::reset() {
  fov = 75.0;
  position = glm::vec3(0.0, 0.0, 1.0);
  front = glm::vec3(0.0, 0.0, -1.0);
  updateProjection();
  updateView();
  updateFrustum();
}

void Camera::updateFrustum() {
  glm::mat4 transform = projection * view;
  GLfloat* m = glm::value_ptr(transform);
  updateFrustumPlane(0, m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]);
  updateFrustumPlane(1, m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]);
  updateFrustumPlane(2, m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]);
  updateFrustumPlane(3, m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]);
  updateFrustumPlane(4, m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]);
  updateFrustumPlane(5, m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]);
}

void Camera::updateFrustumPlane(const GLuint index, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w) {
  frustum[index].normal = glm::vec3(x, y, z);
  const GLfloat inverseNormalLength = 1.0f / glm::length(frustum[index].normal);
  frustum[index].normal *= inverseNormalLength;
  frustum[index].constant = w * inverseNormalLength;
}

void Camera::updateProjection() {
  projection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
  version++;
}

void Camera::updateView() {
  view = glm::lookAt(
    position,
    position + front,
    worldUp
  );
  version++;
}
