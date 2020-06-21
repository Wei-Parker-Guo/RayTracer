#include "materials.h"
#include "shaders/basic_shaders.h"

void Material::apply_shade(vec3 r, const vec3 c, const vec3 l, const vec3 lc, const vec3 p, const Triangle& t) {
	//if we reached this virtual class method then it means no material at all, show an entirely yellow block
	vec3 yellow = { 1.0f, 1.0f, 0.0f };
	vec3_deep_copy(r, yellow);
}

LambertMat::LambertMat(const vec3 base_c, const vec3 ambient_c, const vec3 reflect_c) {
	vec3_deep_copy(this->base_c, base_c);
	vec3_deep_copy(this->ambient_c, ambient_c);
	vec3_deep_copy(this->reflect_c, reflect_c);
}

void LambertMat::apply_shade(vec3 r, const vec3 c, const vec3 l, const vec3 lc, const vec3 p, const Triangle& t){
	gen_lambert_shade(this->ambient_c, this->reflect_c, lc, t.norm, l, r);
}
