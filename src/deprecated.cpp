//this file contains deprecated code that might be of interest to some
//comments on each structure/function/code block will be self-explaining
//this file won't, and shouldn't be compiled for the project, use at your own risk


//*************************************
// DECLARATION
//*************************************

//recursive creation function for node's children, but based on number of objects
BVHNode* create_left_n_right_n(std::vector<BVHNode*>& surfaces);

//recursive creation function for a node's children
BVHNode* create_left_n_right(std::vector<BVHNode*>& surfaces, int AXIS, int depth);

//**************************************
// IMPLEMENTATION
//**************************************

//old test block to run tests for the ray memory allocator
//logprintf("Allocator test: Creating and allocating 10000 rays with page size of 64 rays.\n");
//RayPool ray_pool = RayPool(64);
//for (int i = 0; i < 10000; i++) {
//    Ray* new_r = (Ray*)malloc(sizeof(Ray));
//    new_r->depth = i;
//    ray_pool.push(new_r);
//}
//logprintf("Destroying all of them.\n");
//for (int i = 0; i < 10000; i++) {
//    Ray* r = ray_pool.pop();
//    logprintf("%d", r->depth);
//    free(r);
//}

//old function to import lights' direction from scene
//apply global trans
//aiMatrix4x4 gtrans;
//retrieve_node_gtrans(gtrans, scene, light->mName.C_Str());
//light->mPosition = gtrans * light->mPosition;
//light->mDirection = aiVector3D(0, 0, -1); //reset the local direction because it doesn't comply with maya
//aiQuaternion rot;
//aiVector3D pos;
//gtrans.DecomposeNoScaling(rot, pos);
//light->mDirection = rot.Rotate(light->mDirection) * -1.0f;
//store

BVHNode* create_left_n_right_n(std::vector<BVHNode*>& surfaces) {
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
		result->left = create_left_n_right_n(left_list);
		result->right = create_left_n_right_n(right_list);
		//combine_aabb(result->left->bbox, result->right->bbox, result->bbox);
	}

	return result;
}

BVHNode* create_left_n_right(std::vector<BVHNode*>& surfaces, int AXIS, int depth) {
	BVHNode* result = new BVHNode();

	//base case
	int N = surfaces.size();
	if (N == 1) {
		result->left = surfaces[0];
		result->right = NULL;
		printf("hit one\n");
		copy_box(result->bbox, result->left->bbox);
	}
	else if (N == 2) {
		result->left = surfaces[0];
		result->right = surfaces[1];
		printf("hit two\n");
		combine_aabb(result->left->bbox, result->right->bbox, result->bbox);
	}

	//depth base case to limit stack size of recursion
	else if (depth == 0) {
		//switch to object number partition if we reached the depth
		//result = create_left_n_right_n(surfaces);

		////this one below just groups the undivided into undivided iterative triangle groups, but might be preferred in some situations
		TriangleSet* set = new TriangleSet(surfaces);
		copy_box(result->bbox, set->bbox);
		printf("Stack cap of minimal %d triangles.\n", set->triangles.size());
		result->left = set;
		result->right = NULL;
	}

	//recursive case
	else {
		//figure out lists to split according to AXIS
		copy_box(result->bbox, surfaces[0]->bbox);
		for (int i = 0; i < surfaces.size(); i++) {
			combine_aabb(surfaces[i]->bbox, result->bbox, result->bbox);
		}
		float mid = result->bbox[0][AXIS] + (result->bbox[1][AXIS] - result->bbox[0][AXIS]) / 2;
		std::vector<BVHNode*> left_list;
		std::vector<BVHNode*> right_list;
		while (left_list.size() == 0 || right_list.size() == 0) {
			left_list.clear();
			right_list.clear();
			for (int i = 0; i < surfaces.size(); i++) {
				BVHNode* n = surfaces[i];
				if (n->bbox[1][AXIS] < mid || n->bbox[0][AXIS] < mid) left_list.push_back(n);
				else right_list.push_back(n);
			}
			AXIS = (AXIS + 1) % 3;
		}
		//create the parent
		result->left = create_left_n_right(left_list, (AXIS + 1) % 3, depth - 1);
		result->right = create_left_n_right(right_list, (AXIS + 1) % 3, depth - 1);
		//combine_aabb(result->left->bbox, result->right->bbox, result->bbox);
	}

	return result;
}
