#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include "camera.hpp"
#include "geometry.hpp"
#include "object.hpp"
#include "shader.hpp"
#include "texture.hpp"

class Mesh: public Object {
  public:
    Mesh(Geometry* geometry, Shader* shader);
    ~Mesh();
    btRigidBody* getBody();
    void setBody(btRigidBody* value);
    const GeometryBounds& getBounds();
    bool getFrustumCulling();
    void setFrustumCulling(const bool enabled);
    Geometry* getGeometry();
    Shader* getShader();
    const glm::vec3& getPosition();
    void setPosition(const glm::vec3& value);
    const glm::quat& getRotation();
    void setRotation(const glm::quat& value);
    const glm::vec3& getScale();
    void setScale(const glm::vec3& value);
    void lookAt(const glm::vec3 & target);
    const glm::mat4& getTransform();
    void setUniformInt(const std::string& name, const GLint value);
    void setUniformFloat(const std::string& name, const GLfloat value);
    void setUniformTexture(const std::string& name, Texture* texture);
    void setUniformVec2(const std::string& name, const glm::vec2& value);
    void setUniformVec3(const std::string& name, const glm::vec3& value);
    void setUniformVec4(const std::string& name, const glm::vec4& value);
    void render(Camera* camera, const GLsizei instances = 0);
  private:
    btRigidBody* body;
    GeometryBounds bounds;
    GLuint boundsVersion;
    bool frustumCulling;
    Geometry* geometry;
    Shader* shader;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 transform;
    glm::mat3 normalTransform;
    bool needsBoundsUpdate;
    bool needsTransformUpdate;
    std::map<std::string, Texture*> uniformsTexture;
    std::map<std::string, GLint> uniformsInt;
    std::map<std::string, GLfloat> uniformsFloat;
    std::map<std::string, glm::vec2> uniformsVec2;
    std::map<std::string, glm::vec3> uniformsVec3;
    std::map<std::string, glm::vec4> uniformsVec4;
    void updateTransform();
};
