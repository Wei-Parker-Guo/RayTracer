#include "ray.h"


Ray::Ray() {
    vec3_zero(this->e);
    vec3_zero(this->d);
    this->t = 0;
}

Ray::Ray(const vec3 origin, const vec3 direction, const bool is_dir) {
    vec3 dir;
    vec3_deep_copy(dir, direction);

    //second point mode
    if (!is_dir) {
        vec3_sub(dir, dir, origin);
    }

    vec3_deep_copy(this->e, origin);
    vec3_deep_copy(this->d, dir);
    vec3_norm(this->d, this->d);
    this->t = 0.0f;
}

void Ray::get_point(const float t, vec3 p) const {
    vec3_scale(p, d, t);
    vec3_add(p, e, p);
}

void Ray::reflect(const vec3 norm, const float t, const float epsilon, Ray& ray) {
    vec3 origin;

    //figure out the original p and apply epsilon to find the new origin
    this->get_point(t - epsilon, origin);
    vec3_deep_copy(ray.e, origin);

    //figure out reflect direction
    vec3_reflect(ray.d, this->d, norm);
}
