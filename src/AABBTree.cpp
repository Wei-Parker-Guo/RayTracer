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

	//recursively partition the root node to contain all meshes to hit test, NULL the right to connect with the meshes group
	//convert mesh vector to node vector first
	std::vector<BVHNode*> mesh_nodes;
	for (int i = 0; i < meshes.size(); i++) {
		mesh_nodes.push_back(meshes[i]);
	}
	this->root_node->left = this->create_left_n_right(mesh_nodes, 0);
	this->root_node->right = NULL;

	//recursively parition the triangles in each mesh node into the tree
	for (int i = 0; i < meshes.size(); i++) {
		Mesh* mesh = meshes[i];
		//convert the triangles into nodes for the partition
		std::vector<BVHNode*> triangle_nodes;
		for (int j = 0; i < mesh->unit_surfaces.size(); j++) {
			triangle_nodes.push_back(mesh->unit_surfaces[j]);
		}
		//partition
		mesh->left = create_left_n_right(triangle_nodes, 0);
		mesh->right = NULL;
	}
}

BVHNode* AABBTree::create_left_n_right(std::vector<BVHNode*>& surfaces, int AXIS) {
	BVHNode* result = new BVHNode();

	//base case
	int N = surfaces.size();
	if (N == 1) {
		result->left = surfaces[0];
		result->right = NULL;
		copy_box(result->bbox, result->left->bbox);
	}
	else if (N == 2) {
		result->left = surfaces[0];
		result->right = surfaces[1];
		combine_aabb(result->left->bbox, result->right->bbox, result->bbox);
	}

	//recursive case
	else {
		//figure out lists to split according to AXIS
		box b_total;
		copy_box(b_total, surfaces[0]->bbox);
		for (int i = 1; i < surfaces.size(); i++) {
			combine_aabb(surfaces[i]->bbox, b_total, b_total);
		}
		float mid = b_total[1][AXIS];
		std::vector<BVHNode*> left_list;
		std::vector<BVHNode*> right_list;
		for (int i = 0; i < surfaces.size(); i++) {
			BVHNode* n = surfaces[i];
			if (n->bbox[1][AXIS] <= mid) left_list.push_back(n);
			else right_list.push_back(n);
		}
		//create the parent
		result->left = create_left_n_right(left_list, (AXIS + 1) % 3);
		result->right = create_left_n_right(right_list, (AXIS + 1) % 3);
		combine_aabb(result->left->bbox, result->right->bbox, result->bbox);
	}

	return result;
}
