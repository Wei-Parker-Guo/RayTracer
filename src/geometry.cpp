#include "geometry.h"

void combine_aabb(const box& a, const box& b, box& out) {
    vec3_deep_copy(out[0], a[0]);
    vec3_deep_copy(out[1], a[1]);
    for (int i = 0; i < 3; i++) {
        //get min
        out[0][i] = fmin(out[0][i], b[0][i]);
        //get max
        out[1][i] = fmax(out[1][i], b[1][i]);
    }
}

void copy_box(box& out, const box& in) {
    vec3_deep_copy(out[0], in[0]);
    vec3_deep_copy(out[1], in[1]);
}

void hitrec_deep_copy(hitrec& out, const hitrec& in) {
    out.t = in.t;
    strcpy(out.mesh_id, in.mesh_id);
    vec3_deep_copy(out.norm, in.norm);
}

bool box_hit(box bbox, const Ray& r) {
    float t[9];
    t[1] = (bbox[0][0] - r.e[0]) / r.d[0];
    t[2] = (bbox[1][0] - r.e[0]) / r.d[0];
    t[3] = (bbox[0][1] - r.e[1]) / r.d[1];
    t[4] = (bbox[1][1] - r.e[1]) / r.d[1];
    t[5] = (bbox[0][2] - r.e[2]) / r.d[2];
    t[6] = (bbox[1][2] - r.e[2]) / r.d[2];
    t[7] = fmax(fmax(fmin(t[1], t[2]), fmin(t[3], t[4])), fmin(t[5], t[6]));
    t[8] = fmin(fmin(fmax(t[1], t[2]), fmax(t[3], t[4])), fmax(t[5], t[6]));
    bool result = (t[8] < 0 || t[7] > t[8]) ? false : true;
    return result;
}

bool Surface::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    return true;
}

void Surface::bounding_box(box& b) {

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

BVHNode::BVHNode(std::vector<Surface*> surfaces, int AXIS, int depth) {
    int N = surfaces.size();

    //if we reached the depth cap then directly group the triangles into one node and leave recursion
    if (depth == 0) {
        this->left = new TriangleSet(surfaces);
        this->right = NULL;
        return;
    }

    //base cases
    if (N == 1) {
        this->left = surfaces[0];
        this->right = NULL;
        surfaces[0]->bounding_box(this->bbox);
    }
    else if (N == 2) {
        this->left = surfaces[0];
        this->right = surfaces[1];
        box left_box;
        box right_box;
        surfaces[0]->bounding_box(left_box);
        surfaces[1]->bounding_box(right_box);
        combine_aabb(left_box, right_box, this->bbox);
    }

    //recursive case
    else {
        //find midpoint m of the bounding box of surfaces along AXIS
        box sbox;
        surfaces[0]->bounding_box(sbox);
        for (int i = 0; i < surfaces.size(); i++) {
            box b;
            surfaces[i]->bounding_box(b);
            combine_aabb(sbox, b, sbox);
        }
        float m = sbox[0][AXIS] + (sbox[1][AXIS] - sbox[0][AXIS]) / 2;
        //parition the surface list
        std::vector<Surface*> left_list;
        std::vector<Surface*> right_list;
        int div_dif = INFINITY;
        int best_axis = 0;
        //try to find the best midpoint that will divide the list evenly
        for (int a = 0; a < 3; a++) {
            left_list.clear();
            right_list.clear();
            for (int i = 0; i < surfaces.size(); i++) {
                box b;
                surfaces[i]->bounding_box(b);
                float bm = b[0][AXIS] + (b[1][AXIS] - b[0][AXIS]) / 2;
                if (bm < m) left_list.push_back(surfaces[i]);
                else right_list.push_back(surfaces[i]);
            }
            int this_div_dif = abs((int)left_list.size() - (int)right_list.size()); //figure out the evenness
            //reached a more desirable divison
            if (this_div_dif < div_dif) {
                div_dif = this_div_dif;
                best_axis = AXIS;
            }
            AXIS = (AXIS + 1) % 3;
        }
        AXIS = best_axis;
        //confirm axis and do partition
        left_list.clear();
        right_list.clear();
        for (int i = 0; i < surfaces.size(); i++) {
            box b;
            surfaces[i]->bounding_box(b);
            float bm = b[0][AXIS] + (b[1][AXIS] - b[0][AXIS]) / 2;
            if (bm < m) left_list.push_back(surfaces[i]);
            else right_list.push_back(surfaces[i]);
        }
        //construct children and link them to parent
        //if any list has zero length then just assign NULL to children it points to
        if (left_list.size() == 0) {
            this->left = NULL;
            this->right = new TriangleSet(right_list);
            this->right->bounding_box(this->bbox);
            return;
        }
        else this->left = new BVHNode(left_list, (AXIS + 1) % 3, depth - 1);
        if (right_list.size() == 0) {
            this->right = NULL;
            this->left = new TriangleSet(left_list);
            this->left->bounding_box(this->bbox);
            return;
        }
        else this->right = new BVHNode(right_list, (AXIS + 1) % 3, depth - 1);
        BVHNode* left = (BVHNode*) this->left;
        BVHNode* right = (BVHNode*)this->right;
        combine_aabb(left->bbox, right->bbox, this->bbox);
    }
}

bool BVHNode::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    //get bounding box hit
    bool result = box_hit(this->bbox, r);

    //if not hit directly return
    if (!result) return false;

    //else do a recursive check of its children
    else {
        hitrec left_rec, right_rec;
        bool left_hit;
        bool right_hit;
        //NULL Check
        left_hit = (this->left != NULL) && this->left->hit(r, t0, t1, left_rec);
        right_hit = (this->right != NULL) && this->right->hit(r, t0, t1, right_rec);
        //cases
        if (left_hit && right_hit) {
            if (left_rec.t < right_rec.t) hitrec_deep_copy(rec, left_rec);
            else hitrec_deep_copy(rec, right_rec);
            return true;
        }
        else if (left_hit) {
            hitrec_deep_copy(rec, left_rec);
            return true;
        }
        else if (right_hit) {
            hitrec_deep_copy(rec, right_rec);
            return true;
        }
        else
            return false;
    }
}

void BVHNode::bounding_box(box& b) {
    copy_box(b, this->bbox);
}

Polygon::Polygon(const aiFace* face, const std::vector<Vertex*>& total_vertices, char* id) {
    //record info
    strcpy(this->mesh_id, id);
    //record geometry
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
    if (t<t0 || t>t1) return false; //return immediately if out of range

    //send the ray and detect for intersects
    vec3 p;
    r.get_point(t, p);
    //get count of intersects
    int count = p_in_poly(this->vertices.size(), this->vertices, p);
    if (count) {
        strcpy(rec.mesh_id, this->mesh_id);
        return true;
    }

    return false;
}

Triangle::Triangle(const vec3 a, const vec3 b, const vec3 c, const vec3 an, const vec3 bn, const vec3 cn, char* id) {
    //record info
    strcpy(this->mesh_id, id);
    //record vertex
    vec3_deep_copy(this->a, a);
    vec3_deep_copy(this->b, b);
    vec3_deep_copy(this->c, c);
    vec3_deep_copy(this->an, an);
    vec3_deep_copy(this->bn, bn);
    vec3_deep_copy(this->cn, cn);
    //record surface norm by averaging the three vertices' norm
    vec3_zero(this->norm);
    vec3_add(this->norm, this->an, this->bn);
    vec3_add(this->norm, this->norm, this->cn);
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
    float t = (f * ak_jb + e * jc_al + d * bl_kc) / -m;
    if (t < t0 || t > t1) return false;
    float gamma = (i * ak_jb + h * jc_al + g * bl_kc) / m;
    if (gamma < 0 || gamma > 1) return false;
    float beta = (j * ei_hf + k * gf_di + l * dh_eg) / m;
    if (beta < 0 || beta >(1 - gamma)) return false;
    //we hit, interpolate the normal by barycentric coordinates here to obatin point norm
    float alpha = 1 - gamma - beta;
    vec3 bf;
    vec3 gf;
    vec3 pn;
    vec3_mul_float(pn, this->an, alpha);
    vec3_mul_float(bf, this->bn, beta);
    vec3_mul_float(gf, this->cn, gamma);
    vec3_add(pn, pn, bf);
    vec3_add(pn, pn, gf);
    vec3_norm(pn, pn);
    //log hitrec
    rec.t = t;
    vec3_deep_copy(rec.norm, pn);
    strcpy(rec.mesh_id, this->mesh_id);
    return true;
}

void Triangle::bounding_box(box& b) {
    for (int i = 0; i < 3; i++) {
        //get min
        b[0][i] = fmin(a[i], fmin(this->b[i], this->c[i]));
        //get max
        b[1][i] = fmax(a[i], fmax(this->b[i], this->c[i]));
    }
}

Mesh::Mesh(const aiMesh* mesh, Material* mat) {
    //record info
    std::strcpy(this->id, mesh->mName.C_Str());
    //record material
    this->material = mat;
    //record vertices
    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex* vert = new Vertex();
        //read pos
        aivec_to_vec3(vert->pos, mesh->mVertices[i]);
        //read norm
        aivec_to_vec3(vert->norm, mesh->mNormals[i]);
        //read uv
        if (mesh->HasTextureCoords(0)) {
            aiVector3D uvs = mesh->mTextureCoords[0][i];
            vert->uv[0] = uvs.x;
            vert->uv[1] = uvs.y;
        }
        this->vertices.push_back(vert);
    }
    //record faces
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace* face = &(mesh->mFaces[i]);
        this->faces.push_back(face);
    }
    //generate bounding box, we do it here not bounding box call to save calculation
    vec3_deep_copy(this->aabb[0], this->vertices[0]->pos);
    vec3_deep_copy(this->aabb[1], this->vertices[0]->pos);
    for (int i = 0; i < this->vertices.size(); i++) {
        box comp;
        vec3_deep_copy(comp[0], this->vertices[i]->pos);
        vec3_deep_copy(comp[1], this->vertices[i]->pos);
        combine_aabb(this->aabb, comp, this->aabb);
    }
    this->construct_unit_surfaces();
}

void Mesh::construct_unit_surfaces() {
    for (int i = 0; i < this->faces.size(); i++) {
        aiFace* face = faces[i];
        //construct the face got and figure out if it has been hit, if triangle then use the triangle hit method explicitly
        if (face->mNumIndices == 3) {
            Triangle* surface = new Triangle(this->vertices[face->mIndices[0]]->pos, this->vertices[face->mIndices[1]]->pos, this->vertices[face->mIndices[2]]->pos,
                this->vertices[face->mIndices[0]]->norm, this->vertices[face->mIndices[1]]->norm, this->vertices[face->mIndices[2]]->norm,
                this->id);
            //store
            this->unit_surfaces.push_back(surface);
        }
        //treat all other faces (including convex situations) like a polygon
        else {
            Polygon* surface = new Polygon(face, this->vertices, this->id);
        }
    }
}

bool Mesh::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    //check if the bounding box of this mesh is hit, if not directly return false
    if (!box_hit(this->aabb, r)) return false;
    else return this->root_node->hit(r, t0, t1, rec);

    //the old, not efficient iterative method to check each Triangle's hit
    //for (int i = 0; i < this->unit_surfaces.size(); i++) {
    //    Surface* unit = unit_surfaces[i];
    //    if (unit->hit(r, t0, t1, rec)) return true;
    //}

    //return false;
}

float Mesh::aabb_hit(const Ray& r) {
    float t[9];
    t[1] = (this->aabb[0][0] - r.e[0]) / r.d[0];
    t[2] = (this->aabb[1][0] - r.e[0]) / r.d[0];
    t[3] = (this->aabb[0][1] - r.e[1]) / r.d[1];
    t[4] = (this->aabb[1][1] - r.e[1]) / r.d[1];
    t[5] = (this->aabb[0][2] - r.e[2]) / r.d[2];
    t[6] = (this->aabb[1][2] - r.e[2]) / r.d[2];
    t[7] = fmax(fmax(fmin(t[1], t[2]), fmin(t[3], t[4])), fmin(t[5], t[6]));
    t[8] = fmin(fmin(fmax(t[1], t[2]), fmax(t[3], t[4])), fmax(t[5], t[6]));
    bool result = (t[8] < 0 || t[7] > t[8]) ? -1 : t[7];
    return t[7];
}

void Mesh::bounding_box(box& b) {
    vec3_deep_copy(b[0], this->aabb[0]);
    vec3_deep_copy(b[1], this->aabb[1]);
}

TriangleSet::TriangleSet(std::vector<Surface*> triangles) {
    this->triangles = triangles;
    box sbox;
    for (int i = 0; i < triangles.size(); i++) {
        box b;
        triangles[i]->bounding_box(b);
        combine_aabb(sbox, b, sbox);
    }
    copy_box(this->aabb, sbox);
}

bool TriangleSet::hit(const Ray& r, const float t0, const float t1, hitrec& rec) {
    float oldt = INFINITY;
    bool result = false;
    for (int i = 0; i < this->triangles.size(); i++) {
        Surface* unit = triangles[i];
        hitrec newrec;
        if (unit->hit(r, t0, t1, newrec) && newrec.t < oldt) {
            result = true;
            hitrec_deep_copy(rec, newrec);
            oldt = rec.t;
        }
    }

    return result;
}

void TriangleSet::bounding_box(box& b) {
    copy_box(b, this->aabb);
}
