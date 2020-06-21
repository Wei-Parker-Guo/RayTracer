#ifndef BASIC_SHADERS
#define BASIC_SHADERS

#include <algorithm>
#include "../fast_math.h"
#define PI 3.14159265f

/* generate lambertian shade with another light source
 * equation: c = cr(ca + clmax(0, n.l))
 */
void gen_lambert_shade(const vec3 ca, const vec3 cr, const vec3 cl, const vec3 n, const vec3 l, vec3 c);

/* generate phong shade based on a precalculated lambertian shade
equation: c = clambert + clcp(h.n)^p */
void gen_phong_shade(const vec3 cl, const vec3 cp, const vec3 l, const vec3 e, const vec3 n, const int p, vec3 clambert);

/* generate anisotropic phong shade by dividing the normal into uv directions
and calculating specular based on the Ward Anisotropic Distribution Model (a BRDF), finally adding the lambertian base shade.
Notice this can't use our fast_pow since the exponential is float.
The formula is referenced from this url:
https://en.wikibooks.org/wiki/GLSL_Programming/Unity/Brushed_Metal */
void gen_WARD_anisotropic_phong_shade(const vec3 cl, const vec3 cp, const vec3 l, const vec3 e, const vec3 n, const float pu, const float pv, const float y, vec3 clambert);

#endif
