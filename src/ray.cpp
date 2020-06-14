#include "ray.h"

void Ray::get_point(const float t, vec3 p){
    vec3_scale(p, d, t);
    vec3_add(p, e, p);
}
