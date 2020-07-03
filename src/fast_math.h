#ifndef FAST_MATH
#define FAST_MATH

#include "linmath.h" //include some of the linear math ops provided by GLFW
#include "assimp/Importer.hpp"

float sqr(float x);

/* A faster function for square root. ONLY WORKS for 32 bit float, should be ok for most platforms.
This is directly taken from the url below, and used only for low-precision scenarios in this scope:
https://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi 
Not used currently for better looks of a smoother sphere */
float fast_sqrt(float x);

/* A faster function to calculate float powered an int with
binary storage recursion. Time Complexity: O(logn)
Inspired by geeksforgeeks.com's examples, reedited to work with floats. */
float fast_pow(float x, int y);

/* A simple function to make sure every channel for a color stays in [0-1]
could be disabled for better efficiency */
void vec3_cull(vec3 r);

/* A simple function that uses a vector to fraction another.
Basically times each row with each other. */
void vec3_fraction(vec3 r, const vec3 a, const vec3 b);

//a simple function to make a deep copy of a vec3
void vec3_deep_copy(vec3 r, const vec3 a);

// A simple function that takes an rgb color and transform it to three even channeled grayscale color
void rgb_to_grayscale(vec3 r, const vec3 rgb);

//function to zero out a vector
void vec3_zero(vec3 r);

//function to multiply each element of a vector with a float
void vec3_mul_float(vec3 r, const vec3 a, const float b);

//function to deep copy an aiVector3D to vec3
void aivec_to_vec3(vec3 r, const aiVector3D& v);

//function to deep copy an aiColor into vec3
void aicolor_to_vec3(vec3 r, const aiColor3D& c);

#endif
