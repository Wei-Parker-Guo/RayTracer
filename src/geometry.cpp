#include "geometry.h"

void combine_aabb(const box& a, const box& b, box& out) {
    vec3_deep_copy(out[0], a[0]);
    vec3_deep_copy(out[1], a[1]);
    for (int i = 0; i < 3; i++) {
        //get min
        out[0][i] = std::min(out[0][i], b[0][i]);
        //get max
        out[1][i] = std::max(out[1][i], b[1][i]);
    }
}

void copy_box(box& out, const box& in) {
    vec3_deep_copy(out[0], in[0]);
    vec3_deep_copy(out[1], in[1]);
}

//method to determine whether the geometry is hit by the rat
bool Surface::hit(const Ray& r, const float t0, const float t1, hitrec& rec){
    return true;
}

//method to determine bounding box of the geometry
void Surface::bounding_box(box& b){

}

BVHNode::BVHNode() {
    this->left = NULL;
    this->right = NULL;
    vec3_zero(this->bbox[0]);
    vec3_zero(this->bbox[1]);
}

BVHNode::BVHNode(box& b) {
    this->left = NULL;
    this->right = NULL;
    vec3_deep_copy(this->bbox[0], b[0]);
    vec3_deep_copy(this->bbox[1], b[1]);
}

//quicker implementation of bbox hit test that could be integrated on GPU easily
//referenced and adapted from https://gamedev.stackexchange.com/a/103714/73429
bool BVHNode::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    float t[10];
    t[1] = (this->bbox[0][0] - r.e[0]) / r.d[0];
    t[2] = (this->bbox[1][0] - r.e[0]) / r.d[0];
    t[3] = (this->bbox[0][1] - r.e[1]) / r.d[1];
    t[4] = (this->bbox[1][1] - r.e[1]) / r.d[1];
    t[5] = (this->bbox[0][2] - r.e[2]) / r.d[2];
    t[6] = (this->bbox[1][2] - r.e[2]) / r.d[2];
    t[7] = fmax(fmax(fmin(t[1], t[2]), fmin(t[3], t[4])), fmin(t[5], t[6]));
    t[8] = fmin(fmin(fmax(t[1], t[2]), fmax(t[3], t[4])), fmax(t[5], t[6]));
    t[9] = (t[8] < 0 || t[7] > t[8]) ? false : t[7];
    bool result = t[9];
    //if not hit directly return
    if (!result) return false;
    //else do a recursive check of its children
    else {
        return true;
    }
}

Mesh::Mesh(const aiMesh* mesh) {
    //record vertices
    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex* vert = new Vertex();
        //read pos
        aivec_to_vec3(vert->pos, mesh->mVertices[i]);
        //read norm
        aivec_to_vec3(vert->norm, mesh->mNormals[i]);
        //read uv
        aiVector3D uvs = mesh->mTextureCoords[0][i];
        vert->uv[0] = uvs.x;
        vert->uv[1] = uvs.y;
        this->vertices.push_back(vert);
    }
    //record faces
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace* face = &mesh->mFaces[i];
        this->faces.push_back(face);
    }
    aivec_to_vec3(this->aabb[0], mesh->mAABB.mMin);
    aivec_to_vec3(this->aabb[1], mesh->mAABB.mMax);
    this->construct_unit_surfaces();
}

void Mesh::construct_unit_surfaces() {
    for (int i = 0; i < this->faces.size(); i++) {
        aiFace* face = faces[i];
        UnitSurface* surface;
        //construct the face got and figure out if it has been hit, if triangle then use the triangle hit method explicitly
        if (face->mNumIndices == 3) {
            surface = new Triangle(this->vertices[face->mIndices[0]]->pos, this->vertices[face->mIndices[1]]->pos, this->vertices[face->mIndices[2]]->pos);
        }
        //treat all other faces (including convex situations) like a polygon
        else {
            surface = new Polygon(face, this->vertices);
        }
        //store
        this->unit_surfaces.push_back(surface);
    }
}

bool Mesh::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    //using the aabb tree to find the hit among its unit surfaces
    return this->left->hit(r, t0, t1, rec);

    //the old, not efficient iterative method to check each Triangle's hit
    /*for (int i = 0; i < this->unit_surfaces.size(); i++) {
        Surface* unit = unit_surfaces[i];
        if (unit->hit(r, t0, t1, rec)) return true;
    }

    return false;*/
}

void Mesh::bounding_box(box& b) {
    vec3_deep_copy(b[0], this->aabb[0]);
    vec3_deep_copy(b[1], this->aabb[1]);
}

UnitSurface::UnitSurface() {
    BVHNode::BVHNode();
}

Polygon::Polygon(const aiFace* face, const std::vector<Vertex*>& total_vertices) {
    vec3_zero(this->norm);
    for (int i = 0; i < face->mNumIndices; i++) {
        int index = face->mIndices[i];
        Vertex* vert = new Vertex();
        //deep copy the info
        vec3_deep_copy(vert->norm, total_vertices[index]->norm);
        vec3_deep_copy(vert->pos, total_vertices[index]->pos);
        vec3_deep_copy(vert->uv, total_vertices[index]->uv);
        this->vertices.push_back(vert);
        vec3_add(this->norm, this->norm, vert->norm);
    }
    vec3_scale(this->norm, this->norm, 1 / face->mNumIndices); //average the vertex norms to get the surface norm
    vec3_norm(this->norm, this->norm);
}

//The WRF method to check if a point is inside a ploygon (Jordan Curve Theorem)
//referenced and adapted from https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
int p_in_poly(const int nvert, const std::vector<Vertex*>& vertices, const vec3 p) {
    int i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        if (((vertices[i]->pos[1] > p[1]) != (vertices[j]->pos[1] > p[1])) &&
            (p[0] < (vertices[j]->pos[0] - vertices[i]->pos[0]) * (p[1] - vertices[i]->pos[1]) / (vertices[j]->pos[1] - vertices[i]->pos[1]) + vertices[i]->pos[0]))
            c = !c;
    }
    return c;
}

//this method is implemented by the "project and count intersects" method for the xy plane
bool Polygon::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    //check if the ray cross the polygon plane
    float dn = vec3_mul_inner(r.d, this->norm);
    //if d.n = 0 then it's parallel to the plane and has no intersect, return false immediately
    if (dn == 0) return false;
    vec3 a;
    vec3_sub(a, this->vertices[0]->pos, r.e);
    //get the point on the plane
    float t = vec3_mul_inner(a, this->norm) / dn;
    if (t<t0 || t>t1) return false; //return immediately if our of range

    //send the ray and detect for intersects
    vec3 p;
    r.get_point(t, p);
    //get count of intersects
    int count = p_in_poly(this->vertices.size(), this->vertices, p);
    if (count) {
        rec.push_back(t);
        return true;
    }

    return false;
}

Triangle::Triangle(const vec3 a, const vec3 b, const vec3 c) {
    vec3_deep_copy(this->a, a);
    vec3_deep_copy(this->b, b);
    vec3_deep_copy(this->c, c);
    //calculate surface norm by taking the cross product of two edges
    vec3 edge1;
    vec3 edge2;
    vec3_sub(edge1, this->b, this->a); //dir ab
    vec3_sub(edge2, this->c, this->a); //dir ac
    vec3_mul_cross(this->norm, edge1, edge2);
    vec3_norm(this->norm, this->norm);
    //construct bounding box
    this->bounding_box(this->bbox);
}

bool Triangle::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    //construct the matrix
    float a = this->a[0] - this->b[0];
    float b = this->a[1] - this->b[1];
    float c = this->a[2] - this->b[2];
    float d = this->a[0] - this->c[0];
    float e = this->a[1] - this->c[1];
    float f = this->a[2] - this->c[2];
    float g = r.d[0];
    float h = r.d[1];
    float i = r.d[2];
    float j = this->a[0] - r.e[0];
    float k = this->a[1] - r.e[1];
    float l = this->a[2] - r.e[2];
    //construct candy (reused vals)
    float ei_hf = e * i - h * f;
    float gf_di = g * f - d * i;
    float dh_eg = d * h - e * g;
    float ak_jb = a * k - j * b;
    float jc_al = j * c - a * l;
    float bl_kc = b * l - k * c;

    //actual calculation
    float m = a * ei_hf + b * gf_di + c * dh_eg;
    float t = (f * ak_jb + e * jc_al + d * bl_kc) / -m;
    if (t < t0 || t > t1) return false;
    float gamma = (i * ak_jb + h * jc_al + g * bl_kc) / m;
    if (gamma < 0 || gamma > 1) return false;
    float beta = (j * ei_hf + k * gf_di + l * dh_eg) / m;
    if (beta < 0 || beta >(1 - gamma)) return false;
    //log hitrec
    rec.push_back(t);
    return true;
}

void Triangle::bounding_box(box& b) {
    vec3_deep_copy(b[0], this->a);
    vec3_deep_copy(b[1], this->a);
    for (int i = 0; i < 3; i++) {
        //get min
        b[0][i] = std::min(b[0][i], std::min(this->b[i], this->c[i]));
        //get max
        b[1][i] = std::max(b[1][i], std::max(this->b[i], this->c[i]));
    }
}

TriangleSet::TriangleSet(std::vector<BVHNode*> triangles) {
    this->triangles = triangles;
    copy_box(this->bbox, triangles[0]->bbox);
    for (int i = 0; i < triangles.size(); i++) {
        combine_aabb(triangles[i]->bbox, this->bbox, this->bbox);
    }
}

bool TriangleSet::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    for (int i = 0; i < this->triangles.size(); i++) {
        Surface* unit = triangles[i];
        if (unit->hit(r, t0, t1, rec)) return true;
    }

    return false;
}
