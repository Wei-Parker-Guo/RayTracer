#include "ray.h"

void Ray::get_point(const float t, vec3 p) const {
    vec3_scale(p, d, t);
    vec3_add(p, e, p);
}
