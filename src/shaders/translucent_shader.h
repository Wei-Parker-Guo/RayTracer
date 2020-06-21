#ifndef TRANSLU_SHADER
#define TRANSLU_SHADER

#include "basic_shaders.h"

/* This algorithm DOES NOT implement subsurface scattering, it just linearly blend of BRDF and BTDF:
the light returned from inner surface and outer surface specified by a depth parameter.
Will implement a BSSRDF based translucent shader for subsurface scattering when we have
a working ray tracer. */
void gen_translucent_shade(const vec3 ca, const vec3 cr, const vec3 cl, const vec3 cp, vec3 l, const vec3 e, const vec3 norm, const float p,
float x, float y, const float ds, const float dd, const float depth, vec3 c);

#endif
