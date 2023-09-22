#include "node.hpp"

VoxelSearchNode::VoxelSearchNode(): context(nullptr), x(0), y(0), z(0) {

}

VoxelSearchNode::VoxelSearchNode(VoxelSearchContext* context, const GLint x, const GLint y, const GLint z): context(context), x(x), y(y), z(z) {

}

float VoxelSearchNode::GoalDistanceEstimate(VoxelSearchNode &nodeGoal) {
  return glm::length(glm::vec3(x - nodeGoal.x, y - nodeGoal.y, z - nodeGoal.z));
}

bool VoxelSearchNode::IsGoal(VoxelSearchNode &nodeGoal) {
  return (
    (x == nodeGoal.x) && (y == nodeGoal.y) && (z == nodeGoal.z)
  );
}

bool VoxelSearchNode::GetSuccessors(AStarSearch<VoxelSearchNode> *astarsearch, VoxelSearchNode *parent_node) {
  if (context == nullptr) {
    return true;
  }

  const GLint verticalNeighbors[] = { 0, 1, -1 };
  VoxelSearchNode NewNode;

  for (GLint ny = 0; ny < 3; ny++) {
    for (GLint nz = -1; nz <= 1; nz++) {
      for (GLint nx = -1; nx <= 1; nx++) {
        if (nz == 0 && nx == 0) {
          continue;
        }
        const GLint tx = x + nx;
        const GLint ty = y + verticalNeighbors[ny];
        const GLint tz = z + nz;
        if (
          !(parent_node != nullptr && parent_node->x == tx && parent_node->y == ty && parent_node->z == tz)
          && canStepAt(tx, ty, tz)
        ) {
          NewNode = VoxelSearchNode(context, tx, ty, tz);
          astarsearch->AddSuccessor(NewNode);
        }
      }
    }
  }

  return true;
}

float VoxelSearchNode::GetCost(VoxelSearchNode &successor) {
  return 1.0;
  // return y == successor.y ? 1 : 2;
}

bool VoxelSearchNode::IsSameState(VoxelSearchNode &rhs) {
  return (
    (x == rhs.x) && (y == rhs.y) && (z == rhs.z)
  );
}

size_t VoxelSearchNode::Hash() {
  return std::hash<std::string>{}(std::to_string(x) + ":" + std::to_string(y) + ":" + std::to_string(z));
  // size_t h1 = std::hash<GLint>{}(x);
  // size_t h2 = std::hash<GLint>{}(y);
  // size_t h12 = h1 ^ (h2 << 1);
  // size_t h3 = std::hash<GLint>{}(z);
  // return h12 ^ (h3 << 1);
}

bool VoxelSearchNode::canGoThrough(const GLint x, const GLint y, const GLint z) {
  if (context == nullptr) {
    return false;
  }
  for (GLint h = 0; h < context->height; h++) {
    if (!context->voxels->test(x, y + h, z)) {
      return false;
    }
  }
  return true;
}

bool VoxelSearchNode::canStepAt(const GLint x, const GLint y, const GLint z) {
  if (context == nullptr) {
    return false;
  }
  if (context->voxels->test(x, y - 1, z)) {
    return false;
  }
  return canGoThrough(x, y, z);
}
