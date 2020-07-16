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

PointLight::PointLight(aiLight* light) {
	aivec_to_vec3(this->pos, light->mPosition);
	aicolor_to_vec3(this->color, light->mColorDiffuse);
}

void PointLight::get_dir_c(vec3 r, vec3 c, const vec3 ppos) {
	//get color
	vec3_deep_copy(c, this->color);
	//get direction, we jitter it as well from here for softer shadows
	vec3_sub(r, this->pos, ppos);
	vec3_norm(r, r);
	//jitter
	float jitter_x = 0.01f * (rand() % 200 / 100.0f - 1.0f);
	float jitter_y = 0.01f * (rand() % 200 / 100.0f - 1.0f);
	float jitter_z = 0.01f * (rand() % 200 / 100.0f - 1.0f);
	//apply
	vec3 add_amount = { jitter_x, jitter_y, jitter_z };
	vec3_add(r, r, add_amount);
	vec3_norm(r, r);
}

void PointLight::get_dir(vec3 r, const vec3 ppos) {
	vec3_sub(r, this->pos, ppos);
	vec3_norm(r, r);
	//jitter
	float jitter_x = 0.01f * (rand() % 200 / 100.0f - 1.0f);
	float jitter_y = 0.01f * (rand() % 200 / 100.0f - 1.0f);
	float jitter_z = 0.01f * (rand() % 200 / 100.0f - 1.0f);
	//apply
	vec3 add_amount = { jitter_x, jitter_y, jitter_z };
	vec3_add(r, r, add_amount);
	vec3_norm(r, r);
}
