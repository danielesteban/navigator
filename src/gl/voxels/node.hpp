#pragma once

#include "../../core/lib/stlastar.hpp"
#include "volume.hpp"

struct VoxelSearchContext {
  const GLint height;
  Voxels* voxels;
};

class VoxelSearchNode {
  public:
    VoxelSearchNode();
    VoxelSearchNode(VoxelSearchContext* context, const GLint x, const GLint y, const GLint z);
    float GoalDistanceEstimate(VoxelSearchNode &nodeGoal);
    bool IsGoal(VoxelSearchNode &nodeGoal);
    bool GetSuccessors(AStarSearch<VoxelSearchNode> *astarsearch, VoxelSearchNode *parent_node);
    float GetCost(VoxelSearchNode &successor);
    bool IsSameState(VoxelSearchNode &rhs);
    size_t Hash();
    bool canGoThrough(const GLint x, const GLint y, const GLint z);
    bool canStepAt(const GLint x, const GLint y, const GLint z);

    VoxelSearchContext* context;
    GLint x;
    GLint y;
    GLint z;
};
