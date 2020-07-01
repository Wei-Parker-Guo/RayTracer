#include "AABBTree.h"

BVHNode* create_left_n_right_n(std::vector<BVHNode*>& surfaces) {
	BVHNode* result = new BVHNode();

	//base case
	int N = surfaces.size();
	if (N == 0) {
		return NULL;
	}

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
		copy_box(result->bbox, surfaces[0]->bbox);
		for (int i = 0; i < surfaces.size(); i++) {
			combine_aabb(surfaces[i]->bbox, result->bbox, result->bbox);
		}
		std::vector<BVHNode*>::iterator left_start_i = surfaces.begin();
		std::vector<BVHNode*>::iterator left_end_i = surfaces.begin() + N/2;
		std::vector<BVHNode*>::iterator right_start_i = surfaces.begin() + N / 2 + 1;
		std::vector<BVHNode*>::iterator right_end_i = surfaces.end();
		std::vector<BVHNode*> left_list(left_start_i, left_end_i);
		std::vector<BVHNode*> right_list(right_start_i, right_end_i);
		surfaces.clear(); //release surfaces from stack
		result->left = create_left_n_right_n(left_list);
		result->right = create_left_n_right_n(right_list);
	}

	return result;
}

BVHNode* create_left_n_right(std::vector<BVHNode*>& surfaces, int AXIS, int depth) {
	BVHNode* result = new BVHNode();

	//base case
	int N = surfaces.size();
	if (N == 0) {
		return NULL;
	}

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

	//depth base case to limit stack size of recursion
	else if (depth == 0) {
		//switch to object number partition if we reached the depth
		result = create_left_n_right_n(surfaces);

		//this one below just groups the undivided into undivided iterative triangle groups, but might be preferred in some situations
		/*TriangleSet* set = new TriangleSet(surfaces);
		copy_box(result->bbox, set->bbox);
		printf("Stack cap of minimal %d triangles.\n", set->triangles.size());*/
	}

	//recursive case
	else {
		//figure out lists to split according to AXIS
		copy_box(result->bbox, surfaces[0]->bbox);
		for (int i = 0; i < surfaces.size(); i++) {
			combine_aabb(surfaces[i]->bbox, result->bbox, result->bbox);
		}
		float mid = result->bbox[1][AXIS] / 2;
		std::vector<BVHNode*> left_list;
		std::vector<BVHNode*> right_list;
		for (int i = 0; i < surfaces.size(); i++) {
			BVHNode* n = surfaces[i];
			if (n->bbox[1][AXIS] < mid) left_list.push_back(n);
			else right_list.push_back(n);
		}
		//release some stacks we don't use
		surfaces.clear();
		//create the parent
		result->left = create_left_n_right(left_list, (AXIS + 1) % 3, depth - 1);
		result->right = create_left_n_right(right_list, (AXIS + 1) % 3, depth - 1);
	}

	return result;
}

AABBTree::AABBTree(std::vector<Mesh*>& meshes) {
	//define the rootnode bbox to be containing all the meshes
	box first_mesh_box;
	meshes[0]->bounding_box(first_mesh_box);
	this->root_node = new BVHNode(first_mesh_box);
	for (int i = 0; i < meshes.size(); i++) {
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
	this->root_node->left = create_left_n_right(mesh_nodes, 0, 64);
	this->root_node->right = NULL;

	//recursively parition the triangles in each mesh node into the tree, this subdivision is recorded in the mesh objects
	//the hit function of mesh objects will handle the triangle tree
	for (int i = 0; i < meshes.size(); i++) {
		Mesh* mesh = meshes[i];
		//convert the triangles into nodes for the partition
		std::vector<BVHNode*> triangle_nodes;
		for (int j = 0; j < mesh->unit_surfaces.size(); j++) {
			triangle_nodes.push_back(mesh->unit_surfaces[j]);
		}
		//partition
		mesh->left = create_left_n_right(triangle_nodes, 0, 2048);
		mesh->right = NULL;
	}
}
