#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "camera.hpp"
#include "geometry.hpp"

class Raycaster {
  public:
    Raycaster();
    struct {
      glm::vec3 origin;
      glm::vec3 direction;
    } ray;
    struct {
      GLuint id;
      GLfloat distance;
      glm::vec3 normal;
      glm::vec3 position;
    } result;
    void init();
    void intersect(const GLuint id, const GeometryBounds& bounds, Geometry* geometry, const glm::mat4& transform);
    void setFromCamera(Camera* camera, const glm::vec2& position);
  private:
    bool intersectsBounds(const GeometryBounds &bounds);
    GLfloat intersectTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
    static glm::vec3 transformVector(const glm::vec3& vector, const glm::mat4& matrix);
};
