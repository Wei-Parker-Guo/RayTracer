#include "AABBTree.h"

AABBTree::AABBTree(std::vector<Mesh*> meshes) {
	//define the rootnode bbox to be containing all the meshes
	box first_mesh_box;
	meshes[0]->bounding_box(first_mesh_box);
	this->root_node = new BVHNode(first_mesh_box);
	for (int i = 1; i < meshes.size(); i++) {
		Mesh* mesh = meshes[i];
		box mesh_box;
		mesh->bounding_box(mesh_box);
		combine_aabb(this->root_node->bbox, mesh_box, this->root_node->bbox);
	}

	//recursively partition the root node to contain all elements to hit test
}
