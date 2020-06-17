#include "geometry.h"


//method to determine whether the geometry is hit by the rat
bool Surface::hit(const Ray& r, const float t0, const float t1, hitrec& rec){
    return true;
}

//method to determine bounding box of the geometry
void Surface::bounding_box(box b){

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
    }
    return false;
}

void Mesh::bounding_box(box b) {

}

Triangle::Triangle(const vec3 a, const vec3 b, const vec3 c) {
    vec3_deep_copy(this->a, a);
    vec3_deep_copy(this->b, b);
    vec3_deep_copy(this->c, c);
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
