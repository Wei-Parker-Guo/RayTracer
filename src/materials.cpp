#include "materials.h"
#include "shaders/basic_shaders.h"

void Material::apply_shade(vec3 r, const vec3 p, const vec3 e, const vec3 norm, std::vector<Light*> lights, shadow_ray_rec& sh_rec) {
	//if we reached this virtual class method then it means no material at all, show an entirely yellow block
	vec3 yellow = { 1.0f, 1.0f, 0.0f };
	vec3_deep_copy(r, yellow);
}

LambertMat::LambertMat(aiMaterial* mat) {
	//don't use reflectivity on this mat
	this->use_reflectivity = false;
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

void LambertMat::apply_shade(vec3 r, const vec3 p, const vec3 e, const vec3 norm, std::vector<Light*> lights, shadow_ray_rec& sh_rec) {
	//zero the result
	vec3_zero(r);
	//get light info
	vec3 lc;
	vec3 l;
	for (int i = 0; i < lights.size(); i++) {
		Light* light = lights[i];
		vec3 c_add;
		vec3_zero(c_add);
		light->get_dir_c(l, lc, p);
		gen_lambert_shade(this->ambient_c, this->base_c, lc, norm, l, c_add);
		vec3_mul_float(c_add, c_add, sh_rec.shadow_frac[i]);
		vec3_add(r, r, c_add);
	}
}

PhongMat::PhongMat(aiMaterial* mat) {
	//get base color
	aiColor3D base_color(0.5f, 0.5f, 0.5f); //default diffuse set to gray
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);
	aicolor_to_vec3(this->base_c, base_color);
	//get ambient color
	aiColor3D ambient_color(0.05f, 0.05f, 0.05f); //default ambient set to barely visible dark
	mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient_color);
	aicolor_to_vec3(this->ambient_c, ambient_color);
	//get specular color
	aiColor3D spec_color(0.5f, 0.5f, 0.5f); //default specular set to diffuse default gray to mimic metal
	mat->Get(AI_MATKEY_COLOR_SPECULAR, spec_color);
	aicolor_to_vec3(this->spec_c, spec_color);
	//get shininess
	this->shininess = 32; //default shininess set to 64
	mat->Get(AI_MATKEY_SHININESS, this->shininess);
	//get reflectivity and specify to the renderer to use it
	this->reflectivity = 0.5f; //default reflectivity set to 0.5
	mat->Get(AI_MATKEY_REFLECTIVITY, this->reflectivity);
	this->use_reflectivity = true;
}

void PhongMat::apply_shade(vec3 r, const vec3 p, const vec3 e, const vec3 norm, std::vector<Light*> lights, shadow_ray_rec& sh_rec) {
	//zero the result
	vec3_zero(r);
	//figure out viewing vector
	vec3 view;
	vec3_sub(view, e, p);
	vec3_norm(view, view);
	//get light info
	vec3 lc;
	vec3 l;
	for (int i = 0; i < lights.size(); i++) {
		Light* light = lights[i];
		vec3 c_add;
		vec3_zero(c_add);
		light->get_dir_c(l, lc, p);
		gen_lambert_shade(this->ambient_c, this->base_c, lc, norm, l, c_add);
		gen_phong_shade(lc, this->spec_c, l, view, norm, this->shininess, c_add);
		vec3_mul_float(c_add, c_add, sh_rec.shadow_frac[i]);
		vec3_add(r, r, c_add);
	}
}
