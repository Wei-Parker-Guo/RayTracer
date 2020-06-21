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
    int depth;

    //default constructor to create a zeroed out ray
    Ray();

    //constructor to take a pos and direction, normalize the essentials and generate the ray, default t to zero
    //if is_direction is false then assume the second vector is another point and figure out the direction 
    Ray(const vec3 origin, const vec3 direction, const bool is_dir, const int depth);

    //this method gets a point on the ray according to t given, p = e + td
    void get_point(const float t, vec3 p) const;

    //this method reflect the ray around normal at a given t and epsilon, returning the new ray generated
    void reflect(const vec3 norm, const float t, const float epsilon, Ray& ray);
};

#endif
