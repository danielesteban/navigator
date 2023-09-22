#include "volume.hpp"
#include "node.hpp"

Voxels::Voxels(Physics* physics, Shader* shader):
  Object(),
  isPhysicsEnabled(false),
  physics(physics),
  shader(shader)
{

}

Voxels::~Voxels() {
  for (const auto& [k, v] : data) {
    delete v;
  }
  for (const auto& [k, v] : chunks) {
    btRigidBody* body = v->getBody();
    if (body != nullptr) {
      physics->removeBody(body);
    }
    delete v;
  }
}

void Voxels::enablePhysics() {
  if (isPhysicsEnabled) {
    return;
  }
  for (const auto& [k, chunk] : chunks) {
    if (chunk->getBody() == nullptr) {
      if (chunk->needsCollidersUpdate) {
        chunk->updateColliders();
      }
      physics->addBody(chunk);
    }
  }
}

void Voxels::disablePhysics() {
  if (!isPhysicsEnabled) {
    return;
  }
  for (const auto& [k, chunk] : chunks) {
    btRigidBody* body = chunk->getBody();
    if (body != nullptr) {
      physics->removeBody(body);
    }
  }
}

const std::map<std::string, VoxelChunk*>& Voxels::getChunks() {
  return chunks;
}

Shader* Voxels::getShader() {
  return shader;
}

VoxelChunk::Data* Voxels::getData(const GLint x, const GLint y, const GLint z) {
  std::string key = std::to_string(x) + ":" + std::to_string(y) + ":" + std::to_string(z);
  if (!data.contains(key)) {
    data[key] = new VoxelChunk::Data{};
  }
  return data[key];
}

VoxelChunk* Voxels::getChunk(const GLint x, const GLint y, const GLint z) {
  std::string key = std::to_string(x) + ":" + std::to_string(y) + ":" + std::to_string(z);
  if (!chunks.contains(key)) {
    VoxelChunk* chunk = new VoxelChunk((Object*) this, x, y, z);
    for (GLint i = 0, cz = z - 1; cz <= z; cz++) {
      for (GLint cy = y - 1; cy <= y; cy++) {
        for (GLint cx = x - 1; cx <= x; cx++, i++) {
          chunk->data.at(i) = getData(cx, cy, cz);
        }
      }
    }
    if (isPhysicsEnabled) {
      physics->addBody(chunk);
    }
    chunks[key] = chunk;
  }
  return chunks[key];
}

void Voxels::render(Camera* camera) {
  shader->setCameraUniforms(camera);
  shader->use();
  for (const auto& [key, chunk] : chunks) {
    if (!camera->isInFrustum(chunk->getBounds())) {
      continue;
    }
    shader->setUniformMat4("modelMatrix", chunk->getTransform());
    shader->setUniformMat3("normalMatrix", chunk->getNormalTransform());
    chunk->draw();
  }
}

void Voxels::set(const GLint x, const GLint y, const GLint z, const VoxelType type, const GLubyte r, const GLubyte g, const GLubyte b) {
  GLint cx = floor((GLfloat) x / VoxelChunk::size);
  GLint cy = floor((GLfloat) y / VoxelChunk::size);
  GLint cz = floor((GLfloat) z / VoxelChunk::size);
  GLint vx = x - cx * VoxelChunk::size;
  GLint vy = y - cy * VoxelChunk::size;
  GLint vz = z - cz * VoxelChunk::size;
  GLuint vi = vz * VoxelChunk::size * VoxelChunk::size + vy * VoxelChunk::size + vx;
  Voxel& v = getData(cx, cy, cz)->at(vi);
  const VoxelType current = v.type;
  v = { type, r, g, b };

  bool needsUpdate = !(
    (current == VOXEL_TYPE_AIR && type == VOXEL_TYPE_OBSTACLE)
    || (current == VOXEL_TYPE_OBSTACLE && type == VOXEL_TYPE_AIR)
  );

  const GLint halfChunkSize = VoxelChunk::size / 2;
  for (GLint nz = 0; nz < 2; nz++) {
    for (GLint ny = 0; ny < 2; ny++) {
      for (GLint nx = 0; nx < 2; nx++) {
        if (
          vx >= ((nx * halfChunkSize) - 1) && vx <= (nx + 1) * halfChunkSize
          && vy >= ((ny * halfChunkSize) - 1) && vy <= (ny + 1) * halfChunkSize
          && vz >= ((nz * halfChunkSize) - 1) && vz <= (nz + 1) * halfChunkSize
        ) {
          VoxelChunk* chunk = getChunk(cx + nx, cy + ny, cz + nz);
          if (needsUpdate) {
            chunk->needsUpdate = true;
          }
          chunk->needsCollidersUpdate = true;
        }
      }
    }
  }
}

bool Voxels::ground(const GLint x, const GLint y, const GLint z, const GLint height, GLint& ground) {
  if (!test(x, y, z)) {
    return false;
  }
  for (GLint i = 0, g = y - 1; i < 100; i++, g--) {
    if (!test(x, g, z, VOXEL_TYPE_SOLID)) {
      continue;
    }
    for (GLint h = 1; h <= height; h++) {
      if (!test(x, g + h, z)) {
        return false;
      }
    }
    ground = g + 1;
    return true;
  }
  return false;
}

Voxel Voxels::get(const GLint x, const GLint y, const GLint z) {
  GLint cx = floor((GLfloat) x / VoxelChunk::size);
  GLint cy = floor((GLfloat) y / VoxelChunk::size);
  GLint cz = floor((GLfloat) z / VoxelChunk::size);
  std::string key = std::to_string(cx) + ":" + std::to_string(cy) + ":" + std::to_string(cz);
  if (!data.contains(key)) {
    return Voxel(VOXEL_TYPE_AIR, 0, 0, 0);
  }
  GLint vx = x - cx * VoxelChunk::size;
  GLint vy = y - cy * VoxelChunk::size;
  GLint vz = z - cz * VoxelChunk::size;
  GLuint vi = vz * VoxelChunk::size * VoxelChunk::size + vy * VoxelChunk::size + vx;
  return data[key]->at(vi);
}

bool Voxels::test(const GLint x, const GLint y, const GLint z, const VoxelType type) {
  GLint cx = floor((GLfloat) x / VoxelChunk::size);
  GLint cy = floor((GLfloat) y / VoxelChunk::size);
  GLint cz = floor((GLfloat) z / VoxelChunk::size);
  std::string key = std::to_string(cx) + ":" + std::to_string(cy) + ":" + std::to_string(cz);
  if (!data.contains(key)) {
    return type == VOXEL_TYPE_AIR;
  }
  GLint vx = x - cx * VoxelChunk::size;
  GLint vy = y - cy * VoxelChunk::size;
  GLint vz = z - cz * VoxelChunk::size;
  GLuint vi = vz * VoxelChunk::size * VoxelChunk::size + vy * VoxelChunk::size + vx;
  return data[key]->at(vi).type == type;
}

std::vector<GLint> Voxels::pathfind(
  const GLint fromX,
  const GLint fromY,
  const GLint fromZ,
  const GLint toX,
  const GLint toY,
  const GLint toZ,
  const GLint height
) {
  VoxelSearchContext context(height, this);
  VoxelSearchNode from(&context, fromX, fromY, fromZ);
  VoxelSearchNode to(&context, toX, toY, toZ);
  AStarSearch<VoxelSearchNode> astarsearch(4096);
  astarsearch.SetStartAndGoalStates(from, to);
  GLuint SearchState;
  do {
    SearchState = astarsearch.SearchStep();
    // if (astarsearch.GetStepCount() > 4096) {
    //   astarsearch.CancelSearch();
    // }
  } while (
    SearchState == AStarSearch<VoxelSearchNode>::SEARCH_STATE_SEARCHING
  );

  std::vector<GLint> results;
  if (SearchState == AStarSearch<VoxelSearchNode>::SEARCH_STATE_SUCCEEDED) {
    VoxelSearchNode *node = astarsearch.GetSolutionStart();
    results.push_back(node->x);
    results.push_back(node->y);
    results.push_back(node->z);
    while (node = astarsearch.GetSolutionNext()) {
      results.push_back(node->x);
      results.push_back(node->y);
      results.push_back(node->z);
    }
    astarsearch.FreeSolutionNodes();
  }
  astarsearch.EnsureMemoryFreed();
  return results;
}
