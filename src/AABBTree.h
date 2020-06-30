#ifndef AABBTree_H
#define AABBTree_H

#include <assimp/scene.h>
#include <vector>
#include "geometry.h"

//class representing a binary aabb tree structure, created from an aiscene and optimizes hit tests/queries
class AABBTree {
	public:
		//root node of the tree
		BVHNode* root_node;
		//constructor that takes a list of meshes and arrange the triangles into an aabb binary tree
		AABBTree(std::vector<Mesh*> meshes);
		//recursive creation function for a node's children
		BVHNode* create_left_n_right(std::vector<BVHNode*>& surfaces, int AXIS);
};

#endif
