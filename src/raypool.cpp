#include "raypool.h"
#include <cstdlib>
#include <iostream>

//constructors
RayPool::RayPool(const unsigned int page_size) {
	this->page_size = page_size;
	//create a brand new page as our last page
	this->new_page(NULL);
}

//destructor
RayPool::~RayPool() {
	//free the allocated last page
	free(this->last_page->node);
	free(this->last_page->prev);
	free(this->last_page);
}

//method to create a new blank page and link it to the pool, given pointer to its prev
void RayPool::new_page(RayPage* prev) {
	RayPage* new_page = (RayPage *) malloc(sizeof(RayPage));
	new_page->size = 0;
	new_page->prev = prev;
	new_page->node = NULL;
	this->last_page = new_page;
}

//method to push a ray to the pool
void RayPool::push(Ray* r) {

	//check if our last page is full, if true then create a new page
	if (this->last_page->size == this->page_size) this->new_page(this->last_page);

	//create and add the ray node now
	RayNode* new_node = (RayNode*)malloc(sizeof(RayNode));
	new_node->ray = r;
	new_node->next = this->last_page->node;
	this->last_page->node = new_node;
	this->last_page->size += 1;
	this->total_size += 1;
}

//method to pop out a ray
Ray* RayPool::pop() {

	//determine if we reached the state of an empty last page, if so delete the page and access prev page
 	if (this->last_page->node == NULL) {
		//if we have no more prev pages, just return
		if (this->last_page->prev == NULL) {
			printf("\nStack Underflow\n");
			return NULL;
		}
		else {
			RayPage* tmp = this->last_page;
			this->last_page = this->last_page->prev;
			free(tmp);
		}
	}

	//pop the ray node and destory it, returning the ray
	RayNode* n = this->last_page->node;
	Ray* r = n->ray;
	this->last_page->node = n->next;
	//free the node itself
	free(n);
	this->last_page->size -= 1;
	this->total_size -= 1;
	return r;
}

int RayPool::size() {
	return this->total_size;
}

