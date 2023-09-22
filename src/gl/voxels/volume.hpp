#pragma once

#include "chunk.hpp"
#include "../camera.hpp"
#include "../mesh.hpp"
#include "../object.hpp"
#include "../shader.hpp"
#include "../../core/physics.hpp"
#include <map>
#include <string>

class Voxels: public Object {
  public:
    Voxels(Physics* physics, Shader* shader);
    ~Voxels();
    void enablePhysics();
    void disablePhysics();
    const std::map<std::string, VoxelChunk*>& getChunks();
    Shader* getShader();
    void render(Camera* camera);
    Voxel get(const GLint x, const GLint y, const GLint z);
    void set(const GLint x, const GLint y, const GLint z, const VoxelType type, const GLubyte r, const GLubyte g, const GLubyte b);
    bool ground(const GLint x, const GLint y, const GLint z, const GLint height, GLint& ground);
    bool test(const GLint x, const GLint y, const GLint z, const VoxelType type = VOXEL_TYPE_AIR);
    std::vector<GLint> pathfind(
      const GLint fromX,
      const GLint fromY,
      const GLint fromZ,
      const GLint toX,
      const GLint toY,
      const GLint toZ,
      const GLint height
    );
  private:
    VoxelChunk* getChunk(const GLint x, const GLint y, const GLint z);
    VoxelChunk::Data* getData(const GLint x, const GLint y, const GLint z);
    std::map<std::string, VoxelChunk::Data*> data;
    std::map<std::string, VoxelChunk*> chunks;
    bool isPhysicsEnabled;
    Physics* physics;
    Shader* shader;
};
