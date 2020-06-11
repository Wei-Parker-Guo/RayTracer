#ifndef SKETCH_SHADER
#define SKETCH_SHADER

#include "fast_math.h"
#include "basic_shaders.h"

/* This shader generates a sketch shade based on line direction given as a 2d vector and
a BRDF describing light reflection.
It performs line connection based on midpoint algorithm to connect the dots. */
void gen_sketch_shade(const vec3 ca, const vec3 cr, const vec3 cl, const vec3 cp, vec3 l, const vec3 e, const vec3 norm, const float p,
const float x, const float y, const float radius, vec2 line_dir, vec3 c);

#endif
