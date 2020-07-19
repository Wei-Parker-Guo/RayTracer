#include "ray.h"


Ray::Ray() {
    vec3_zero(this->e);
    vec3_zero(this->d);
    this->t = 0;
    this->depth = 0;
}

Ray::Ray(const vec3 origin, const vec3 direction, const bool is_dir, const int depth, const int id, const ray_type type) {
    //record info
    this->id = id;
    this->type = type;

    //figure out geometry info
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
    this->depth = depth;
}

void Ray::get_point(const float t, vec3 p) const {
    vec3_scale(p, d, t);
    vec3_add(p, e, p);
}

void Ray::reflect(const vec3 norm, const float t, Ray& ray) {
    //figure out reflect direction
    vec3_reflect(ray.d, this->d, norm);

    //figure out the original p and apply epsilon to find the new origin
    vec3 origin;
    this->get_point(t, origin);
    vec3_deep_copy(ray.e, origin);
}

void Ray::split(std::vector<Ray*> out_rays, const int n, const float epsilon) {
    for (Ray* ray : out_rays) {
        //jitter amounts
        float jitter_x = epsilon * (rand() % 200 / 100.0f - 1.0f);
        float jitter_y = epsilon * (rand() % 200 / 100.0f - 1.0f);
        float jitter_z = epsilon * (rand() % 200 / 100.0f - 1.0f);
        //apply
        vec3 add_amount = { jitter_x, jitter_y, jitter_z };
        vec3_add(ray->d, this->d, add_amount);
        vec3_norm(ray->d, ray->d);
        //stack the rest of the info
        vec3_deep_copy(ray->e, this->e);
        ray->id = this->id;
    }
}

float Ray::refrac(const vec3 norm, const float n, const float nt, const float t, Ray& ray) {
    //copy origin
    vec3 ppos;
    this->get_point(t, ppos);
    vec3_deep_copy(ray.e, ppos);

    //figure out fresnel term first, if result is negative then shortcircuit this function (total internal reflection)
    float r0 = sqr((nt - 1) / (nt + 1));
    float r_theta = r0 + (1 - r0) * fast_pow(1 - vec3_mul_inner(this->d, norm), 5);
    if (r_theta < 0) return r_theta; //total internal reflection

    //figure out direction (normalized)
    vec3 term1;
    vec3 term2;
    //first term
    vec3_mul_float(term1, norm, -1 * vec3_mul_inner(this->d, norm));
    vec3_add(term1, this->d, term1);
    vec3_mul_float(term1, term1, n / nt);
    //second term
    vec3_mul_float(term2, norm, sqrt(1 - sqr(n / nt) * (1 - sqr(vec3_mul_inner(d, norm)))));
    //cascade into return dir and normalize
    vec3_sub(ray.d, term1, term2);
    vec3_norm(ray.d, ray.d);
    return r_theta;
}
