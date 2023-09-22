#include "plane.hpp"

PlaneGeometry::PlaneGeometry(const GLfloat width, const GLfloat height): Geometry() {
  const GLfloat hw = width * 0.5;
  const GLfloat hh = height * 0.5;

  vertices.push_back({
    {-hw, -hh, 0.0},  {0.0, 0.0, 1.0},  {0.0, 0.0},  {1.0, 1.0, 1.0},
  });
  vertices.push_back({
    {hw, -hh, 0.0},   {0.0, 0.0, 1.0},  {1.0, 0.0},  {1.0, 1.0, 1.0},
  });
  vertices.push_back({
    {hw, hh, 0.0},    {0.0, 0.0, 1.0},  {1.0, 1.0},  {1.0, 1.0, 1.0},
  });
  vertices.push_back({
    {-hw, hh, 0.0},   {0.0, 0.0, 1.0},  {0.0, 1.0},  {1.0, 1.0, 1.0},
  });

  index.push_back(0);
  index.push_back(1);
  index.push_back(2);
  index.push_back(2);
  index.push_back(3);
  index.push_back(0);

  colliders.push_back({
    GEOMETRY_COLLIDER_BOX,
    glm::vec3(0, 0, -1),
    glm::vec3(hw, hh, 1)
  });
}
