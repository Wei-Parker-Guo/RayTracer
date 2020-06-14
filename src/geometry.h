#ifndef GEOMETRY
#define GEOMETRY

/* This file contains classes representing a 3D geometry for ray traced
rendering. It specifies how each geometry detects hits and other essential
behaviors, including bound boxes for efficiency. */

#include "ray.h"
#include <vector>

typedef std::vector<float> hitrec;
typedef vec3 box[2];

//abstract class for any 3D geometry shapes
class Surface {
    public:

    //method to determine whether the geometry is hit by the rat
    virtual bool hit(const Ray r, const float t0, const float t1, hitrec rec);

    //method to determine bounding box of the geometry
    virtual void bounding_box(box b);
    
};

#endif
