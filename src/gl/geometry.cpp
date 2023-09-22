#include "geometry.hpp"
#include <glm/gtc/type_ptr.hpp>

GLuint Geometry::geometryId = 1;

Geometry::Geometry():
  id(geometryId++),
  refs(1),
  bounds({ glm::vec3(0.0, 0.0, 0.0 ), 0.0 }),
  count(0),
  isValid(false),
  needsUpdate(true),
  needsUpload(false),
  version(1)
{
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
}

Geometry::~Geometry() {
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
}

void Geometry::gc(Geometry* geometry) {
  geometry->refs--;
  if (geometry->refs == 0) {
    delete geometry;
  }
}

GeometryBounds Geometry::getBounds(const glm::mat4& transform) {
  if (needsUpdate) {
    update();
  }
  glm::vec4 position = transform * glm::vec4(bounds.position, 1.0);
  GeometryBounds transformed(
    glm::vec3(position.x, position.y, position.z) / position.w,
    bounds.radius * getMaxScaleOnAxis(transform)
  );
  return transformed;
}

const GLuint Geometry::getVersion() {
  return version;
}

GLfloat Geometry::getMaxScaleOnAxis(const glm::mat4& transform) {
  const GLfloat* t = glm::value_ptr(transform);
  const GLfloat scaleXSq = t[0] * t[0] + t[1] * t[1] + t[2] * t[2];
  const GLfloat scaleYSq = t[4] * t[4] + t[5] * t[5] + t[6] * t[6];
  const GLfloat scaleZSq = t[8] * t[8] + t[9] * t[9] + t[10] * t[10];
  return sqrt(fmax(fmax(scaleXSq, scaleYSq), scaleZSq));
}

void Geometry::draw(const GLsizei instances) {
  if (needsUpdate) {
    update();
  }
  if (!isValid) {
    return;
  }
  if (needsUpload) {
    upload();
  }
  glBindVertexArray(vao);
  if (instances > 0) {
    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0, instances);
  } else {
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
  }
  glBindVertexArray(0);
}

void Geometry::update() {
  needsUpdate = false;
  isValid = false;
  const size_t numIndices = index.size();
  const size_t numVertices = vertices.size();
  if (numIndices == 0 || numVertices == 0) {
    return;
  }
  glm::vec3 max(std::numeric_limits<GLfloat>::min(), std::numeric_limits<GLfloat>::min(), std::numeric_limits<GLfloat>::min());
  glm::vec3 min(std::numeric_limits<GLfloat>::max(), std::numeric_limits<GLfloat>::max(), std::numeric_limits<GLfloat>::max());
  for (const auto& i : index) {
    if (i >= numVertices) {
      return;
    }
    glm::vec3& v = vertices.at(i).position;
    max = glm::max(max, v);
    min = glm::min(min, v);
  }
  bounds.position = (max + min) * (GLfloat) 0.5;
  bounds.radius = glm::distance(max, min) * 0.5;
  isValid = bounds.radius > 0 && index.size() > 0;
  needsUpload = true;
  version++;
}

void Geometry::upload() {
  needsUpload = false;
  count = index.size();
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * index.size(), index.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GeometryVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void *) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void *) (sizeof(GLfloat) * 3));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void *) (sizeof(GLfloat) * 6));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void *) (sizeof(GLfloat) * 8));
  glBindVertexArray(0);
}
