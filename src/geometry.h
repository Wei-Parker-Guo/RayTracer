#ifndef GEOMETRY
#define GEOMETRY

/* This file contains classes representing a 3D geometry for ray traced
rendering. It specifies how each geometry detects hits and other essential
behaviors, including bound boxes for efficiency. */

#include "ray.h"
#include <vector>
#include <assimp/mesh.h>

typedef std::vector<float> hitrec;
typedef vec3 box[2];

//abstract class for any 3D geometry shapes
class Surface {
    public:

    //method to determine whether the geometry is hit by the rat
    virtual bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);

    //method to determine bounding box of the geometry
    virtual void bounding_box(box b);
    
};

struct Vertex {
    vec3 pos;
    vec3 norm;
    vec2 uv;
};

enum texture_type {
    base_color,
    specular,
    metallic,
    height,
    bump,
    roughness
};

struct Texture {
    unsigned int id;
    texture_type type;
};

//class representing a mesh
class Mesh : public Surface {
    private:
        std::vector<Vertex> vertices;
        std::vector<aiFace> faces;
        std::vector<Texture> textures;

    public:
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
        void bounding_box(box b);
};

//class representing an n-vert polygon
class Polygon : public Surface {
    private:
        std::vector<Vertex> vertices;

    public:
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
};

//class representing a triangle
class Triangle : public Surface {
    private:
        //coordinates of the triangle, global space right handed
        vec3 a;
        vec3 b;
        vec3 c;

    public:
        Triangle(const vec3 a, const vec3 b, const vec3 c);
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
};

#endif
