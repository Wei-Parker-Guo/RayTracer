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

//function to return a combined bounding box including two boxes
void combine_aabb(const box& a, const box& b, box& out);

//function to copy a bounding box to another
void copy_box(box& out, const box& in);

//abstract class for any 3D geometry shapes
class Surface {
    public:

    //method to determine whether the geometry is hit by the rat
    virtual bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);

    //method to determine bounding box of the geometry
    virtual void bounding_box(box& b);
    
};

//class representing a bounding box node
class BVHNode : public Surface {
    public:
    BVHNode* left;
    BVHNode* right;
    box bbox;

    //constructor to create an empty node with zero bounding box
    BVHNode();
    //constructor to create a childless node with bounding box given
    BVHNode(box& b);
    //hit function of the node, which will just be checking if the ray hits the bounding box if its an empty node
    virtual bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
};

struct Vertex {
    vec3 pos;
    vec3 norm;
    vec2 uv;
};

//general virtual class respresenting a unit surface type that could form a mesh, often a triangle or polygon
class UnitSurface : public BVHNode {
public:
    UnitSurface();
};

//class representing a mesh
class Mesh : public BVHNode {
    private:
        std::vector<Vertex*> vertices;
        std::vector<aiFace*> faces;
        box aabb;

    public:
        std::vector<UnitSurface*> unit_surfaces; //unit surfaces forming the mesh, could be triangles/polygons
        Mesh(const aiMesh* mesh);
        void construct_unit_surfaces(); //construct the unit surfaces from scratch with vertex and faces given
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);

        void bounding_box(box& b);
};

//class representing an n-vert polygon
class Polygon : public UnitSurface {
    private:
        std::vector<Vertex*> vertices;
        vec3 norm;

    public:
        //a constructor that takes a face and total vertices of a mesh to construct the polygon
        Polygon(const aiFace* face, const std::vector<Vertex*> &total_vertices);
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
};

//class representing a triangle
class Triangle : public UnitSurface {
    public:
        //coordinates of the triangle, global space right handed
        vec3 a;
        vec3 b;
        vec3 c;
        vec3 norm;

    public:
        Triangle(const vec3 a, const vec3 b, const vec3 c);
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
        void bounding_box(box& b);
};

#endif
