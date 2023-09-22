#include "box.hpp"
#include <glm/gtx/rotate_vector.hpp>

BoxGeometry::BoxGeometry(const GLfloat width, const GLfloat height, const GLfloat depth): Geometry() {
  const GLfloat hw = width * 0.5;
  const GLfloat hh = height * 0.5;
  const GLfloat hd = depth * 0.5;

  struct Face {
    GLfloat angle;
    glm::vec3 axis;
    glm::vec3 normal;
  };
  const Face faces[] = {
    { glm::pi<GLfloat>() * 0.5,  { 0, 1, 0 }, { 1, 0, 0 } },
    { glm::pi<GLfloat>() * -0.5, { 0, 1, 0 }, { -1, 0, 0 } },
    { glm::pi<GLfloat>() * -0.5, { 1, 0, 0 }, { 0, 1, 0 } },
    { glm::pi<GLfloat>() * 0.5,  { 1, 0, 0 }, { 0, -1, 0 } },
    { 0,                         { 0, 1, 0 }, { 0, 0, 1 } },
    { glm::pi<GLfloat>(),        { 0, 1, 0 }, { 0, 0, -1 } },
  };
  for (GLint f = 0, i = 0; f < 6; f++, i+=4) {
    const Face face = faces[f];
    vertices.push_back({
      glm::rotate({-hw, -hh, hd}, face.angle, face.axis),  face.normal,  {0.0, 0.0},  {1.0, 1.0, 1.0},
    });
    vertices.push_back({
      glm::rotate({hw, -hh, hd}, face.angle, face.axis),   face.normal,  {1.0, 0.0},  {1.0, 1.0, 1.0},
    });
    vertices.push_back({
      glm::rotate({hw, hh, hd}, face.angle, face.axis),    face.normal,  {1.0, 1.0},  {1.0, 1.0, 1.0},
    });
    vertices.push_back({
      glm::rotate({-hw, hh, hd}, face.angle, face.axis),   face.normal,  {0.0, 1.0},  {1.0, 1.0, 1.0},
    });
    index.push_back(i + 0);
    index.push_back(i + 1);
    index.push_back(i + 2);
    index.push_back(i + 2);
    index.push_back(i + 3);
    index.push_back(i + 0);
  }

  colliders.push_back({
    GEOMETRY_COLLIDER_BOX,
    glm::vec3(0, 0, 0),
    glm::vec3(hw, hh, hd)
  });
}
