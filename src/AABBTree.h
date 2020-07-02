#ifndef AABBTree_H
#define AABBTree_H

#include <vector>
#include "geometry.h"

//class representing a binary aabb tree structure, created from an aiscene and optimizes hit tests/queries
class AABBTree {
	public:
		//list of meshes
		std::vector<Mesh*> meshes;
		//a rootnode for all the meshes only
		BVHNode* mesh_root;
		//constructor that takes a list of meshes and arrange the triangles into an aabb binary tree
		AABBTree(std::vector<Mesh*>& meshes);
};

#endif
