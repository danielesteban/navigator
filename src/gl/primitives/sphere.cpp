#include "sphere.hpp"
#include <vector>

static int getMiddlePoint(std::vector<glm::vec3>& vertices, int p1, int p2) {
  glm::vec3 point1 = vertices[p1];
  glm::vec3 point2 = vertices[p2];
  glm::vec3 middle = glm::vec3((point1.x + point2.x) / 2.0, (point1.y + point2.y) / 2.0, (point1.z + point2.z) / 2.0);
  double length = sqrt(middle.x * middle.x + middle.y * middle.y + middle.z * middle.z);
  vertices.push_back(glm::vec3(middle.x / length, middle.y / length, middle.z / length));
  int i = vertices.size() - 1;
  return i;
}

SphereGeometry::SphereGeometry(const GLfloat radius): Geometry() {
  std::vector<GLushort> _indices;
  std::vector<glm::vec3> _vertices;

  GLushort elements[] = {
    0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8, 3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9, 4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
  };
  
  for (int i = 0; i < 60; i++) {
    _indices.push_back(elements[i]);
  }

  double t = (1.0 + sqrt(5.0)) / 2.0;
  _vertices.push_back(glm::vec3(-1.0, t, 0.0));
  _vertices.push_back(glm::vec3(1.0, t, 0.0));
  _vertices.push_back(glm::vec3(-1.0, -t, 0.0));
  _vertices.push_back(glm::vec3(1.0, -t, 0.0));
  _vertices.push_back(glm::vec3(0.0, -1.0, t));
  _vertices.push_back(glm::vec3(0.0, 1.0, t));
  _vertices.push_back(glm::vec3(-0.0, -1.0, -t));
  _vertices.push_back(glm::vec3(-0.0, 1.0, -t));
  _vertices.push_back(glm::vec3(t, 0.0, -1.0));
  _vertices.push_back(glm::vec3(t, 0.0, 1.0));
  _vertices.push_back(glm::vec3(-t, 0.0, -1.0));
  _vertices.push_back(glm::vec3(-t, 0.0, 1.0));

  for (int i = 0; i < _vertices.size(); i++) {
    double length = sqrt(_vertices[i].x * _vertices[i].x + _vertices[i].y * _vertices[i].y + _vertices[i].z * _vertices[i].z);
    _vertices[i] = glm::vec3(_vertices[i].x / length, _vertices[i].y / length, _vertices[i].z / length);
  }

  const int NUM_REVISIONS = 5;
  for (int i = 0; i < NUM_REVISIONS; i++) {
    std::vector<glm::vec3> _vertices2;
    std::vector<GLushort> _indices2;

    for (int j = 0; j < _indices.size(); j += 3) {
      unsigned int a = getMiddlePoint(_vertices, _indices[j], _indices[j + 1]);
      unsigned int b = getMiddlePoint(_vertices, _indices[j + 1], _indices[j + 2]);
      unsigned int c = getMiddlePoint(_vertices, _indices[j + 2], _indices[j]);

      _vertices2.push_back(_vertices[_indices[j]]);
      _indices2.push_back(j * 2);
      _vertices2.push_back(_vertices[a]);
      _indices2.push_back((j * 2) + 1);
      _vertices2.push_back(_vertices[c]);
      _indices2.push_back((j * 2) + 2);
      _vertices2.push_back(_vertices[_indices[j + 1]]);
      _indices2.push_back((j * 2) + 3);
      _vertices2.push_back(_vertices[b]);
      _indices2.push_back((j * 2) + 4);
      _indices2.push_back((j * 2) + 1);
      _vertices2.push_back(_vertices[_indices[j + 2]]);
      _indices2.push_back((j * 2) + 5);
      _indices2.push_back((j * 2) + 2);
      _indices2.push_back((j * 2) + 4);
      _indices2.push_back((j * 2) + 1);
      _indices2.push_back((j * 2) + 4);
      _indices2.push_back((j * 2) + 2);
    }

    _vertices = _vertices2;
    _indices = _indices2;
  }

  for (const auto& vertex : _vertices) {
    vertices.push_back({
      vertex * radius,
      glm::normalize(vertex),
      // @dani @incomplete: Compute UVs
      glm::vec2(0.0, 0.0),
      glm::vec3(1.0, 1.0, 1.0)
    });
  }

  index = _indices;

  colliders.push_back({
    GEOMETRY_COLLIDER_SPHERE,
    glm::vec3(0, 0, 0),
    glm::vec3(radius, radius, radius)
  });
}
