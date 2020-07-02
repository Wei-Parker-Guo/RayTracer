#include "AABBTree.h"

AABBTree::AABBTree(std::vector<Mesh*>& meshes) {
	this->meshes = meshes;

	//recursively partition the triangles in each mesh node into the tree, this subdivision root is recorded in the mesh objects
	//the hit function of mesh objects will handle the triangle tree
	std::vector<Surface*> mesh_surfaces;
	for (Mesh* mesh : meshes) {
		mesh->root_node = new BVHNode(mesh->unit_surfaces, 0, 1024);
		mesh_surfaces.push_back(mesh);
	}

	//recursively parition the meshes
	this->mesh_root = new BVHNode(mesh_surfaces, 0, 1024);
}
