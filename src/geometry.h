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

//abstract class for any 3D geometry shapes
class Surface {
    public:

    //method to determine whether the geometry is hit by the rat
    virtual bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);

    //method to determine bounding box of the geometry
    virtual void bounding_box(box& b);
    
};

//class representing a bounding box node
class BVHNode {
    public:
    Surface* left;
    Surface* right;
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

//class representing a mesh
class Mesh : public Surface, public BVHNode {
    private:
        std::vector<Vertex*> vertices;
        std::vector<aiFace*> faces;
        std::vector<Surface*> unit_surfaces; //unit surfaces forming the mesh, could be triangles/polygons
        box aabb;

    public:
        Mesh(const aiMesh* mesh);
        void construct_unit_surfaces(); //construct the unit surfaces from scratch with vertex and faces given
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);

        void bounding_box(box& b);
};

//class representing an n-vert polygon
class Polygon : public Surface, public BVHNode {
    private:
        std::vector<Vertex*> vertices;
        vec3 norm;

    public:
        //a constructor that takes a face and total vertices of a mesh to construct the polygon
        Polygon(const aiFace* face, const std::vector<Vertex*> &total_vertices);
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
};

//class representing a triangle
class Triangle : public Surface, public BVHNode {
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
