#include "geometry.h"

//method to determine whether the geometry is hit by the rat
bool Surface::hit(const Ray& r, const float t0, const float t1, hitrec& rec){
    return true;
}

//method to determine bounding box of the geometry
void Surface::bounding_box(box b){

}

Mesh::Mesh(const aiMesh* mesh) {
    //record vertices
    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vert;
        //read pos
        aivec_to_vec3(vert.pos, mesh->mVertices[i]);
        //read norm
        aivec_to_vec3(vert.norm, mesh->mNormals[i]);
        //read uv
        aiVector3D uvs = mesh->mTextureCoords[0][i];
        vert.uv[0] = uvs.x;
        vert.uv[1] = uvs.y;
        this->vertices.push_back(vert);
    }
    //record faces
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        this->faces.push_back(face);
    }
    aivec_to_vec3(this->aabb[0], mesh->mAABB.mMin);
    aivec_to_vec3(this->aabb[1], mesh->mAABB.mMax);
}

bool Mesh::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    //TO-DO: use bounding box to short chain this algorithm


    for (int i = 0; i < this->faces.size(); i++) {
        aiFace face = faces[i];
        //construct the face got and figure out if it has been hit, if triangle then use the triangle hit method explicitly
        if (face.mNumIndices == 3) {
            Triangle triangle = Triangle(this->vertices[face.mIndices[0]].pos, this->vertices[face.mIndices[1]].pos, this->vertices[face.mIndices[2]].pos);
            bool hit = triangle.hit(r, t0, t1, rec);
            if (hit) return true;
        }
        //treat all other faces (including convex situations) like a polygon
        else {
            Polygon polygon = Polygon(face, this->vertices);
            bool hit = polygon.hit(r, t0, t1, rec);
            if (hit) return true;
        }
    }
    return false;
}

void Mesh::bounding_box(box b) {
    vec3_deep_copy(b[0], this->aabb[0]);
    vec3_deep_copy(b[1], this->aabb[1]);
}

Polygon::Polygon(const aiFace& face, const std::vector<Vertex>& total_vertices) {
    vec3_zero(this->norm);
    for (int i = 0; i < face.mNumIndices; i++) {
        int index = face.mIndices[i];
        Vertex vert;
        //deep copy the info
        vec3_deep_copy(vert.norm, total_vertices[index].norm);
        vec3_deep_copy(vert.pos, total_vertices[index].pos);
        vec3_deep_copy(vert.uv, total_vertices[index].uv);
        this->vertices.push_back(vert);
        vec3_add(this->norm, this->norm, vert.norm);
    }
    vec3_scale(this->norm, this->norm, 1 / face.mNumIndices); //average the vertex norms to get the surface norm
    vec3_norm(this->norm, this->norm);
}

//The WRF method to check if a point is inside a ploygon (Jordan Curve Theorem)
//referenced and adapted from https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
int p_in_poly(const int nvert, const std::vector<Vertex>& vertices, const vec3 p) {
    int i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        if (((vertices[i].pos[1] > p[1]) != (vertices[j].pos[1] > p[1])) &&
            (p[0] < (vertices[j].pos[0] - vertices[i].pos[0]) * (p[1] - vertices[i].pos[1]) / (vertices[j].pos[1] - vertices[i].pos[1]) + vertices[i].pos[0]))
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
    vec3_sub(a, this->vertices[0].pos, r.e);
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
    float t = (f * ak_jb + e * jc_al + d * bl_kc) / m * (-1.0f);
    if (t < t0 || t > t1) return false;
    float gamma = (i * ak_jb + h * jc_al + g * bl_kc) / m;
    if (gamma < 0 || gamma > 1) return false;
    float beta = (j * ei_hf + k * gf_di + l * dh_eg) / m;
    if (beta < 0 || beta >(1 - gamma)) return false;
    //log hitrec
    rec.push_back(t);
    return true;
}
