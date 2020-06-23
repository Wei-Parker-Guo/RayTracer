#include "lights.h"

void Light::get_dir_c(vec3 r, vec3 c, const vec3 ppos) {

}

void DirectLight::get_dir_c(vec3 r, vec3 c, const vec3 ppos) {
	vec3_deep_copy(c, this->color);
	vec3_deep_copy(r, this->direction); //this assumes we have normalized the direction
}
