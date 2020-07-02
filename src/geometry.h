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

//abstract base class of any geometry concept
class Surface {
    public:
        //vitual method to check a hit with ray
        virtual bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
        //vitual method to determine bounding box of the geometry
        virtual void bounding_box(box& b);
};

//abstract class representing a bounding box node
class BVHNode : Surface {
    public:
    Surface* left;
    Surface* right;
    box bbox;

    //constructor to create an empty node with zero bounding box
    BVHNode();
    //constructor to create a childless node with bounding box given
    BVHNode(box& b);
    //constructor to recursively create BVHNodes from a list of surfaces (the tree itself)
    //the axis are used to patition the left and right list for the children on reach recursion call
    BVHNode(std::vector<Surface*> surfaces, int AXIS, int depth);

    //recursive hit function of the node, which will be checking if the ray hits the bounding box if its an empty node
    virtual bool hit(const Ray& r, const float t0, const float t1, hitrec& rec);
    //method to determine bounding box of the geometry
    virtual void bounding_box(box& b);
};

struct Vertex {
    vec3 pos;
    vec3 norm;
    vec2 uv;
};

//class representing an n-vert polygon
class Polygon : public Surface {
private:
    std::vector<Vertex*> vertices;
    vec3 norm;

public:
    //a constructor that takes a face and total vertices of a mesh to construct the polygon
    Polygon(const aiFace* face, const std::vector<Vertex*>& total_vertices);
    bool hit(const Ray& r, const float t0, const float t1, hitrec& rec) override;
};

//class representing a triangle
class Triangle : public Surface {
public:
    //coordinates of the triangle, global space right handed
    vec3 a;
    vec3 b;
    vec3 c;
    vec3 norm;

public:
    Triangle(const vec3 a, const vec3 b, const vec3 c);
    bool hit(const Ray& r, const float t0, const float t1, hitrec& rec) override;
    void bounding_box(box& b) override;
};

//class representing a mesh
class Mesh : public Surface {
    private:
        std::vector<Vertex*> vertices;
        std::vector<aiFace*> faces;

    public:
        box aabb;
        BVHNode* root_node; // root node of the triangle trees in this mesh
        std::vector<Surface*> unit_surfaces; //unit surfaces forming the mesh, could be triangles/polygons
        Mesh(const aiMesh* mesh);
        void construct_unit_surfaces(); //construct the unit surfaces from scratch with vertex and faces given
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec) override;

        void bounding_box(box& b) override;
};

//class representing a set of Triangles
class TriangleSet : public Surface {
    public:
        //list of triangles
        std::vector<Surface*> triangles;
        //bounding box
        box aabb;
        //constructor to create a set given a list of triangles
        TriangleSet(std::vector<Surface*> triangles);
        //the hit test just tests each triangle iteratively
        bool hit(const Ray& r, const float t0, const float t1, hitrec& rec) override;
        void bounding_box(box& b) override;
};

#endif
