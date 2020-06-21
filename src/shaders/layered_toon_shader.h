#ifndef LAYERED_TOON_SHADER
#define LAYERED_TOON_SHADER

#include "../fast_math.h"

// A simple function that layers a color with ratio given
void vec3_layer(vec3 r, const vec3 maxr, const float max_n, const float n);

/* generate cartoon like artistic shade based linear blend of a warm color and a cold color
it also takes a phong highlight base and linearly layered the specular to put on top of basic shade
with respect to light source. cc2 is warm cc1 is cold. It also takes a layer param for rendering layered speculars. */
void gen_toon_shade(const vec3 cc1, const vec3 cc2, const vec3 l, const vec3 cl, const vec3 cp, const float toonl, const vec3 e, const vec3 norm, vec3 cphong);

#endif
