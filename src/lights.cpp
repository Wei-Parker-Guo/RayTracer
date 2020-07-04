#include "lights.h"

void Light::get_dir_c(vec3 r, vec3 c, const vec3 ppos) {}

void Light::get_dir(vec3 r, const vec3 ppos){}

DirectLight::DirectLight(aiLight* light) {
	aivec_to_vec3(this->direction, light->mDirection);
	vec3_norm(this->direction, this->direction); //normalize the light
	aicolor_to_vec3(this->color, light->mColorDiffuse);
}

void DirectLight::get_dir_c(vec3 r, vec3 c, const vec3 ppos) {
	vec3_deep_copy(c, this->color);
	vec3_deep_copy(r, this->direction); //this assumes we have normalized the direction
}

void DirectLight::get_dir(vec3 r, const vec3 ppos) {
	vec3_deep_copy(r, this->direction);
}