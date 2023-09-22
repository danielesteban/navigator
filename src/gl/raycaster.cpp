#include "raycaster.hpp"

Raycaster::Raycaster() {

}

void Raycaster::init() {
  result.id = 0;
  result.distance = std::numeric_limits<GLfloat>::max();
  result.position = glm::vec3(0, 0, 0);
}

void Raycaster::intersect(const GLuint id, const GeometryBounds& bounds, Geometry* geometry, const glm::mat4& transform) {
  if (!geometry->isValid || !intersectsBounds(bounds)) {
    return;
  }
  for (size_t i = 0, l = geometry->index.size(); i < l; i += 3) {
    glm::vec3 a = transformVector(geometry->vertices.at(geometry->index.at(i)).position, transform);
    glm::vec3 b = transformVector(geometry->vertices.at(geometry->index.at(i + 1)).position, transform);
    glm::vec3 c = transformVector(geometry->vertices.at(geometry->index.at(i + 2)).position, transform);
    GLfloat d = intersectTriangle(a, b, c);
    if (d != 0.0 && result.distance > d) {
      result.id = id;
      result.distance = d;
      result.normal = glm::normalize(glm::cross(c - b, a - b));
      result.position = ray.origin + ray.direction * d;
    }
  }
}

void Raycaster::setFromCamera(Camera* camera, const glm::vec2& position) {
  ray.origin = camera->getPosition();
  ray.direction = transformVector(glm::vec3(position.x, position.y, 0.5), glm::inverse(camera->getProjection() * camera->getView()));
  ray.direction = glm::normalize(ray.direction - ray.origin);
}

bool Raycaster::intersectsBounds(const GeometryBounds &bounds) {
  GLfloat distance;
  GLfloat directionDistance = glm::dot(bounds.position - ray.origin, ray.direction);
  if (directionDistance < 0) {
    distance = glm::distance(ray.origin, bounds.position);
  } else {
    distance = glm::distance(
      (ray.direction * directionDistance) + ray.origin,
      bounds.position
    );
  }
  return distance <= bounds.radius;
}

GLfloat Raycaster::intersectTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
  const glm::vec3 edge1 = b - a;
  const glm::vec3 edge2 = c - a;
  const glm::vec3 normal = glm::cross(edge1, edge2);
  const GLfloat DdN = -glm::dot(ray.direction, normal);
  if (DdN <= 0.0) return 0.0;
  const glm::vec3 diff = ray.origin - a;
  const GLfloat DdQxE2 = -glm::dot(ray.direction, glm::cross(diff, edge2));
  if (DdQxE2 < 0.0) return 0.0;
  const GLfloat DdE1xQ = -glm::dot(ray.direction, glm::cross(edge1, diff));
  if (DdE1xQ < 0.0 || (DdQxE2 + DdE1xQ) > DdN) return 0.0;
  const GLfloat QdN = glm::dot(diff, normal);
  if (QdN < 0.0) return 0.0;
  return QdN / DdN;
}

glm::vec3 Raycaster::transformVector(const glm::vec3& vector, const glm::mat4& matrix) {
  glm::vec4 transformed = matrix * glm::vec4(vector.x, vector.y, vector.z, 1.0);
  return glm::vec3(transformed.x, transformed.y, transformed.z) / transformed.w;
}
