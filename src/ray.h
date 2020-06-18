#ifndef RAY
#define RAY

#include "fast_math.h"

/* This file contains objects representing rays
 */

class Ray {
    public:
    vec3 e;
    vec3 d;
    float t;

    //this method gets a point on the ray according to t given, p = e + td
    void get_point(const float t, vec3 p) const;
};

#endif
