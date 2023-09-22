#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

struct GeometryBounds {
  glm::vec3 position;
  GLfloat radius;
};

enum GeometryColliderShape {
  GEOMETRY_COLLIDER_BOX,
  GEOMETRY_COLLIDER_CAPSULE,
  GEOMETRY_COLLIDER_CYLINDER,
  GEOMETRY_COLLIDER_SPHERE,
};

static const char* GeometryColliderShapeNames[] = {
  "box",
  "capsule",
  "cylinder",
  "sphere",
  nullptr
};

struct GeometryCollider {
  GeometryColliderShape shape;
  glm::vec3 position;
  glm::vec3 scale;
};

struct GeometryVertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
  glm::vec3 color;
};

class Geometry {
  public:
    const GLuint id;
    GLuint refs;
    Geometry();
    virtual ~Geometry();
    static void gc(Geometry* geometry);
    GeometryBounds getBounds(const glm::mat4& transform);
    const GLuint getVersion();
    void draw(const GLsizei instances = 0);
    std::vector<GeometryCollider> colliders;
    std::vector<GLushort> index;
    std::vector<GeometryVertex> vertices;
    bool isValid;
    bool needsUpdate;
  protected:
    GeometryBounds bounds;
    bool needsUpload;
    GLuint version;
    virtual void update();
  private:
    static GLuint geometryId;
    GLuint count;
    GLuint ebo;
    GLuint vao;
    GLuint vbo;
    static GLfloat getMaxScaleOnAxis(const glm::mat4& transform);
    void upload();
};
