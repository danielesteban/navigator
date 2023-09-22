#include "chunk.hpp"
#include <glm/gtc/matrix_inverse.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

const struct {
  GLfloat angle;
  glm::vec3 axis;
  glm::vec3 n;
  glm::vec3 u;
  glm::vec3 v;
} faces[6] = {
  { 0,                         { 0, 1, 0 }, { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } },
  { glm::pi<GLfloat>() * -0.5, { 1, 0, 0 }, { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 } },
  { glm::pi<GLfloat>() * 0.5,  { 1, 0, 0 }, { 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 } },
  { glm::pi<GLfloat>() * -0.5, { 0, 1, 0 }, { -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 } },
  { glm::pi<GLfloat>() * 0.5,  { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 } },
  { glm::pi<GLfloat>(),        { 0, 1, 0 }, { 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 } },
};

static const struct {
  glm::vec3 p;
  glm::vec2 uv;
  glm::vec3 n;
} faceVertices[4] = {
  { {-0.5,0.5,0.5}, { 0, 1 }, { -1, 1, 0 } },
  { {0.5,0.5,0.5}, { 1, 1 }, { 1, 1, 0 } },
  { {-0.5,-0.5,0.5}, { 0, 0 }, { -1, -1, 0 } },
  { {0.5,-0.5,0.5}, { 1, 0 }, { 1, -1, 0 } },
};

static const GLushort faceIndices[2][6] = {
  { 0, 2, 1, 2, 3, 1 },
  { 2, 3, 0, 3, 1, 0 },
};

std::array<bool, VoxelChunk::size * VoxelChunk::size * VoxelChunk::size> VoxelChunk::collidersMap;

VoxelChunk::VoxelChunk(Object* volume, const GLint x, const GLint y, const GLint z):
  Geometry(),
  body(nullptr),
  needsCollidersUpdate(false),
  volume(volume)
{
  needsUpdate = false;
  position = glm::vec3(x, y, z) * (GLfloat) size + ((GLfloat) size * (GLfloat) -0.5);
  transform = glm::translate(glm::mat4(1.0), position);
  normalTransform = glm::inverseTranspose(glm::mat3(transform));
}

btRigidBody* VoxelChunk::getBody() {
  return body;
}

const GeometryBounds& VoxelChunk::getBounds() {
  if (needsUpdate) {
    update();
  }
  return bounds;
}

void VoxelChunk::setBody(btRigidBody* value) {
  body = value;
}

const glm::vec3& VoxelChunk::getPosition() {
  return position;
}

const glm::mat4& VoxelChunk::getTransform() {
  return transform;
}

const glm::mat3& VoxelChunk::getNormalTransform() {
  return normalTransform;
}

Object* VoxelChunk::getVolume() {
  return volume;
}

static GLfloat SRGBToLinear(const GLubyte c) {
  const GLfloat n = (GLfloat) c / 255.0;
	return (n < 0.04045) ? n * 0.0773993808 : pow(n * 0.9478672986 + 0.0521327014, 2.4);
}

void VoxelChunk::update() {
  needsUpdate = false;
  index.clear();
  vertices.clear();
  glm::vec3 max(std::numeric_limits<GLfloat>::min(), std::numeric_limits<GLfloat>::min(), std::numeric_limits<GLfloat>::min());
  glm::vec3 min(std::numeric_limits<GLfloat>::max(), std::numeric_limits<GLfloat>::max(), std::numeric_limits<GLfloat>::max());
  GLuint i = 0;
  for (GLint z = 0; z < size; z++) {
    for (GLint y = 0; y < size; y++) {
      for (GLint x = 0; x < size; x++) {
        const Voxel& voxel = get(x, y, z);
        if (voxel.type != VOXEL_TYPE_SOLID) {
          continue;
        }
        bool isVisible = false;
        for (GLuint f = 0; f < 6; f++) {
          const auto& face = faces[f];
          const glm::vec3 n = glm::vec3(x, y, z) + face.n;
          if (get(n.x, n.y, n.z).type != VOXEL_TYPE_SOLID) {
            GLfloat ao[4];
            const glm::vec3 col = glm::vec3(SRGBToLinear(voxel.r), SRGBToLinear(voxel.g), SRGBToLinear(voxel.b));
            const glm::vec3 pos = glm::vec3(x + 0.5, y + 0.5, z + 0.5);
            for (GLuint v = 0; v < 4; v++) {
              const auto& vertex = faceVertices[v];
              const glm::vec3 vu = face.u * vertex.n.x;
              const glm::vec3 vv = face.v * vertex.n.y;
              ao[v] = getAO(
                get(n.x + vu.x, n.y + vu.y, n.z + vu.z).type == VOXEL_TYPE_SOLID,
                get(n.x + vv.x, n.y + vv.y, n.z + vv.z).type == VOXEL_TYPE_SOLID,
                get(n.x + vu.x + vv.x, n.y + vu.y + vv.y, n.z + vu.z + vv.z).type == VOXEL_TYPE_SOLID
              );
              vertices.push_back({
                pos + glm::rotate(vertex.p, face.angle, face.axis), face.n, vertex.uv, col * (GLfloat) (1.0 - ao[v]),
              });
            }
            const auto& indices = faceIndices[
              (ao[2] + ao[1] > ao[3] + ao[0]) ? 1 : 0
            ];
            for (GLuint vi = 0; vi < 6; vi++) {
              index.push_back(i + indices[vi]);
            }
            i += 4;
            isVisible = true;
          }
        }
        if (isVisible) {
          max = glm::max(max, glm::vec3(x + 1, y + 1, z + 1));
          min = glm::min(min, glm::vec3(x, y, z));
        }
      }
    }
  }
  bounds.position = (max + min) * (GLfloat) 0.5;
  bounds.radius = glm::distance(max, min) * 0.5;
  isValid = bounds.radius > 0 && index.size() > 0;
  needsUpload = isValid;
  bounds = Geometry::getBounds(transform);
  version++;
}

void VoxelChunk::updateColliders() {
  needsCollidersUpdate = false;
  colliders.clear();
  std::fill(collidersMap.begin(), collidersMap.end(), false);
  for (GLint z = 0; z < size; z++) {
    for (GLint y = 0; y < size; y++) {
      for (GLint x = 0; x < size; x++) {
        if (
          get(x, y, z).type == VOXEL_TYPE_AIR
          || collidersMap.at(z * size * size + y * size + x)
        ) {
          continue;
        }

        GLint width, height, depth;
        for (GLint i = z + 1; i <= size; i++) {
          if (
            i == size
            || get(x, y, i).type == VOXEL_TYPE_AIR
            || collidersMap.at(i * size * size + y * size + x)
          ) {
            depth = i - z;
            break;
          }
        }

        height = size - y;
        for (GLint i = z; i < z + depth; i++) {
          for (GLint j = y + 1; j <= y + height; j++) {
            if (
              j == size
              || get(x, j, i).type == VOXEL_TYPE_AIR
              || collidersMap.at(i * size * size + j * size + x)
            ) {
              height = j - y;
            }
          }
        }

        width = size - x;
        for (GLint i = z; i < z + depth; i++) {
          for (GLint j = y; j < y + height; j++) {
            for (GLint k = x + 1; k <= x + width; k++) {
              if (
                k == size
                || get(k, j, i).type == VOXEL_TYPE_AIR
                || collidersMap.at(i * size * size + j * size + k)
              ) {
                width = k - x;
              }
            }
          }
        }

        for (GLint i = z; i < z + depth; i++) {
          for (GLint j = y; j < y + height; j++) {
            for (GLint k = x; k < x + width; k++) {
              collidersMap.at(i * size * size + j * size + k) = true;
            }
          }
        }

        colliders.push_back({
          GEOMETRY_COLLIDER_BOX,
          glm::vec3(x, y, z) + glm::vec3(width, height, depth) * (GLfloat) 0.5,
          glm::vec3(width, height, depth) * (GLfloat) 0.5
        });
      }
    }
  }
}

const Voxel& VoxelChunk::get(const GLint x, const GLint y, const GLint z) {
  GLint chunkX = 0;
  GLint voxelX = x + size / 2;
  if (voxelX >= size) {
    chunkX = 1;
    voxelX -= size;
  }
  GLint chunkY = 0;
  GLint voxelY = y + size / 2;
  if (voxelY >= size) {
    chunkY = 1;
    voxelY -= size;
  }
  GLint chunkZ = 0;
  GLint voxelZ = z + size / 2;
  if (voxelZ >= size) {
    chunkZ = 1;
    voxelZ -= size;
  }
  const GLuint chunk = chunkZ * 4 + chunkY * 2 + chunkX;
  const GLuint voxel = voxelZ * size * size + voxelY * size + voxelX;
  return data.at(chunk)->at(voxel);
}

const GLfloat VoxelChunk::getAO(const bool n1, const bool n2, const bool n3) {
  GLfloat ao = 0.0;
  if (n1) ao += 0.2;
  if (n2) ao += 0.2;
  if ((!n1 || !n2) && n3) ao += 0.2;
  return ao;
}
