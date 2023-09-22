#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "geometry.hpp"

class Camera {
  public:
    Camera();
    void lookAt(const glm::vec3& value);
    void setAspect(const GLfloat value);
    const GLfloat getFov();
    const glm::vec3& getFront();
    void setFov(const GLfloat value);
    const glm::vec3& getPosition();
    void setPosition(const glm::vec3& value);
    const glm::mat4& getProjection();
    const glm::mat4& getView();
    const GLuint getVersion();
    bool isInFrustum(const GeometryBounds& bounds);
    void reset();
  private:
    GLfloat aspect;
    glm::vec3 front;
    struct {
      glm::vec3 normal;
      GLfloat constant;
    } frustum[6];
    GLfloat fov;
    glm::vec3 position;
    glm::mat4 projection;
    glm::mat4 view;
    GLuint version;
    void updateFrustum();
    void updateFrustumPlane(const GLuint index, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w);
    void updateProjection();
    void updateView();
    static const GLfloat nearPlane;
    static const GLfloat farPlane;
    static const glm::vec3 worldUp;
};
