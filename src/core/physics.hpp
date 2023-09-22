#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "../gl/geometry.hpp"
#include "../gl/mesh.hpp"
#include "../gl/voxels/chunk.hpp"

class Physics {
  public:
    Physics();
    void step(GLfloat delta);
    void addBody(Mesh* mesh, const GLfloat mass, const bool isAlwaysActive, const bool isKinematic);
    void addBody(VoxelChunk* chunk);
    btRigidBody* addBody(const std::vector<GeometryCollider>& colliders, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const GLfloat mass = 0.0, const bool isAlwaysActive = false, const bool isKinematic = false);
    void removeBody(btRigidBody* body);
    void setBodyPosition(btRigidBody* body, const glm::vec3& position);
    void setBodyRotation(btRigidBody* body, const glm::quat& rotation);
    btCollisionObject* getTempCollider(const GeometryColliderShape shape, const glm::vec3& position, const glm::vec3& scale);
    std::vector<GLuint> getContactIds(btCollisionObject* target, const GLubyte mask);
    glm::vec3 getAccumulatedContacts(btCollisionObject* target, const GLubyte mask);
    void setGravity(const glm::vec3& gravity);
  private:
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* broadphase;
    btConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    btGhostObject ghost;
    btTransform transform;
    static bool getBodyData(btCollisionObject* target, GLuint& id, GLbyte& flags);
    static btCollisionShape* getColliderShape(const GeometryColliderShape shape, const glm::vec3& scale);
};
