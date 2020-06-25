#ifndef RAYPOOL
#define RAYPOOL

#include "ray.h"

struct RayNode {
	Ray* ray;
	RayNode* next;
};

struct RayPage {
	RayNode* node; //last node in the stack
	int size; //current size of this page
	RayPage* prev; //pointer to previous page
};

//class to schedule rays and manage them
class RayPool {

	private:
		//each page of the stack is managed as a stack, with reference to all pages stored inside
		//LIFO stack is implemented using linked list
		unsigned int page_size = 64; //max size of a page created
		unsigned int total_size = 0;
		RayPage* last_page; //pointer to the last page

		//method to create a new blank page and link it to the pool, given pointer to its prev
		void new_page(RayPage* prev);

	public:
		//constructors
		RayPool(const unsigned int page_size);
		//method to push a ray to the pool
		void push(Ray * r);
		//method to pop out a ray and destroy the node
		Ray* pop();
		//method to retrieve the entire size of the pool
		int size();
};

#endif // !RAYPOOL
