#include "materials.h"
#include "shaders/basic_shaders.h"

void Material::apply_shade(vec3 r, const vec3 p, const vec3 norm, std::vector<Light*> lights) {
	//if we reached this virtual class method then it means no material at all, show an entirely yellow block
	vec3 yellow = { 1.0f, 1.0f, 0.0f };
	vec3_deep_copy(r, yellow);
}

LambertMat::LambertMat(aiMaterial* mat) {
	//get base color
	aiColor3D base_color(0.5f, 0.5f, 0.5f); //default diffuse set to gray
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);
	aicolor_to_vec3(this->base_c, base_color);
	//get ambient color
	aiColor3D ambient_color(0.05f, 0.05f, 0.05f); //default ambient set to barely visible dark
	mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient_color);
	aicolor_to_vec3(this->ambient_c, ambient_color);
}

LambertMat::LambertMat(const vec3 base_color, const vec3 ambient_color) {
	vec3_deep_copy(this->base_c, base_color);
	vec3_deep_copy(this->ambient_c, ambient_color);
}

void LambertMat::apply_shade(vec3 r, const vec3 p, const vec3 norm, std::vector<Light*> lights) {
	//zero the result
	vec3_zero(r);
	//get light info
	vec3 lc;
	vec3 l;
	for (Light* light : lights) {
		vec3 c_add;
		vec3_zero(c_add);
		light->get_dir_c(l, lc, p);
		gen_lambert_shade(this->ambient_c, this->base_c, lc, norm, l, c_add);
		vec3_add(r, r, c_add);
	}
}
