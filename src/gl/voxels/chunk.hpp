#pragma once

#include "../geometry.hpp"
#include "../object.hpp"
#include <array>
#include <btBulletDynamicsCommon.h>

enum VoxelType: GLubyte {
  VOXEL_TYPE_AIR,
  VOXEL_TYPE_SOLID,
  VOXEL_TYPE_OBSTACLE,
};

struct Voxel {
  VoxelType type;
  GLubyte r;
  GLubyte g;
  GLubyte b;
};

class VoxelChunk : public Geometry {
  public:
    static const GLint size = 16;
    typedef std::array<Voxel, size * size * size> Data;
    std::array<Data*, 8> data;
    VoxelChunk(Object* volume, const GLint x, const GLint y, const GLint z);
    btRigidBody* getBody();
    void setBody(btRigidBody* value);
    const GeometryBounds& getBounds();
    const glm::vec3& getPosition();
    const glm::mat4& getTransform();
    const glm::mat3& getNormalTransform();
    Object* getVolume();
    void update();
    void updateColliders();
    bool needsCollidersUpdate;
  private:
    btRigidBody* body;
    glm::vec3 position;
    glm::mat4 transform;
    glm::mat3 normalTransform;
    Object* volume;
    const Voxel& get(const GLint x, const GLint y, const GLint z);
    static const GLfloat getAO(const bool n1, const bool n2, const bool n3);
    static std::array<bool, size * size * size> collidersMap;
};
