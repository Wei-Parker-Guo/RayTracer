#ifndef RAY
#define RAY

#include "fast_math.h"
#include <vector>

/* This file contains objects representing rays
 */

enum class ray_type {
    shadow,
    reflect
};

class Ray {
    public:
        int id;
        ray_type type;
        vec3 e;
        vec3 d;
        float t;
        int depth;

        //a color cache to store the returned color
        vec3 c_cache;
        //a shadow cache to store the shadow contrib
        float sh_cache;
        //a contribution percentage for this ray's hit color return
        float contrib;
        //a contribution percentage for previous total contrib (used if we hit a nonreflective so we can know the ray's return color contrib)
        float total_previous_contrib;

        //default constructor to create a zeroed out ray
        Ray();

        //constructor to take a pos and direction, normalize the essentials and generate the ray, default t to zero
        //if is_direction is false then assume the second vector is another point and figure out the direction 
        Ray(const vec3 origin, const vec3 direction, const bool is_dir, const int depth, const int id, const ray_type type);

        //this method gets a point on the ray according to t given, p = e + td
        void get_point(const float t, vec3 p) const;

        //this method reflect the ray around normal at a given t and epsilon, returning the new ray generated
        void reflect(const vec3 norm, const float t, const float epsilon, Ray& ray);

        //this method splits the ray into a list of subrays by monte carlo method, bounded by an epsilon range
        void split(std::vector<Ray*> out_rays, const int n, const float epsilon);
};

#endif
