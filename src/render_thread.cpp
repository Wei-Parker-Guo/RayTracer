#include "render_thread.h"

const float PI = 3.1415926;

bool get_hit(const AABBTree& aabb_tree, const Ray* ray, const float t0, const float t1, hitrec& rec) {
    bool hit = false;
    //get hit
    rec.t = INFINITY;
    //[TO-DO] Optimize this by checking the bounding box of mesh to determine which one to do more hit tests
    for (int i = 0; i < aabb_tree.meshes.size(); i++) {
        Mesh* mesh = aabb_tree.meshes[i];
        hitrec newrec;
        if (mesh->hit(*ray, t0, t1, newrec)) {
            hit = true;
            if ((newrec.t < rec.t)) {
                rec.t = newrec.t;
                strcpy(rec.mesh_id, newrec.mesh_id);
                vec3_deep_copy(rec.norm, newrec.norm);
            }
        }
    }
    return hit;
}

void RenderThread::operator()(Rasterizer* rasterizer, AABBTree& aabb_tree, Camera* use_cam, std::vector<Light*> lights,
    const int startX, const int startY, const int endX, const int endY,
    const int ray_pool_page_size, const float set_hfov, const int samples_per_pixel, const int samples_per_ray, const int max_ray_bounce, const float epsilon, const float ray_eps,
    const float max_refrac_bounce) {

    //initialize a ray allocator
    RayPool* ray_pool = new RayPool(ray_pool_page_size);

    //figure out constant parameters for the viewport
    const float aspect_ratio = rasterizer->getHeight() / (float)rasterizer->getWidth();
    const float d = use_cam->near_clip;
    const float a = d * tan(PI * set_hfov / 360); //note that we prefer our user hfov here instead of value from imported camera since assimp has trouble loading it
    const float l = -a;
    const float r = a;
    const float t = a * aspect_ratio;
    const float b = -t;

    int ray_id = 0;
    for (int i = startX; i < endX; i++) {
        for (int j = startY; j < endY; j++) {

            //***********************
            // OPERATION PIXEL
            //***********************

            //a new color sequence for each pixel
            colorseq cs;

            //**************************
            //RAY POOL INITIALIZATION
            //**************************
            //loop over to generate sample rays on a pixel
            for (int x = 0; x < samples_per_pixel; x++) {
                for (int y = 0; y < samples_per_pixel; y++) {
                    float unit_len = 1.0f / samples_per_pixel;

                    //jitter the center of the ray
                    float jitter_x = rand() % 100 / 100.0f;
                    float jitter_y = rand() % 100 / 100.0f;

                    //figure out uvs
                    float u = l + (r - l) * (i + unit_len * x + jitter_x * unit_len) / rasterizer->getWidth();
                    float v = b + (t - b) * (j + unit_len * y + jitter_y * unit_len) / rasterizer->getHeight();

                    //figure out ray
                    vec3 dir_u;
                    vec3 dir_v;
                    vec3 dir_w;
                    vec3 ray_dir;
                    vec3_scale(dir_u, use_cam->side, u);
                    vec3_scale(dir_v, use_cam->up, v);
                    vec3_scale(dir_w, use_cam->lookat, -d);
                    vec3_add(ray_dir, dir_u, dir_v);
                    vec3_add(ray_dir, ray_dir, dir_w);
                    vec3_norm(ray_dir, ray_dir);

                    //construct render ray
                    Ray* new_render_ray = (Ray*)malloc(sizeof(Ray));
                    new_render_ray->id = ray_id;
                    new_render_ray->type = ray_type::reflect;
                    vec3_deep_copy(new_render_ray->e, use_cam->pos);
                    vec3_deep_copy(new_render_ray->d, ray_dir);
                    new_render_ray->depth = max_ray_bounce;
                    new_render_ray->contrib = 1;
                    new_render_ray->total_previous_contrib = 0;
                    new_render_ray->refraci = 1; //assume we start from each pixel in air
                    new_render_ray->weight = 1;
                    vec3_zero(new_render_ray->c_cache);
                    //add to pool
                    ray_pool->push(new_render_ray);

                    //construct shadow ray
                    Ray* new_shadow_ray = (Ray*)malloc(sizeof(Ray));
                    new_shadow_ray->id = ray_id;
                    new_shadow_ray->type = ray_type::shadow;
                    new_shadow_ray->depth = max_ray_bounce;
                    vec3_deep_copy(new_shadow_ray->e, use_cam->pos);
                    vec3_deep_copy(new_shadow_ray->d, ray_dir);
                    //add to pool
                    ray_pool->push(new_shadow_ray);

                    //update ray id
                    ray_id += 1;
                }
            }

            //*********************************************
            //SUBRAY GENERATION & RETURN COLOR CALCULATION
            //*********************************************

            shadow_ray_rec shadow_rec;
            vec3 weight_sum; //weight sum for averaging colors later
            vec3_zero(weight_sum);

            while (ray_pool->size() != 0) {
                Ray* ray = ray_pool->pop();
                hitrec rec;
                bool hit = get_hit(aabb_tree, ray, epsilon, INFINITY, rec);

                //handle the info and calculate color
                vec3 c;
                vec3_zero(c);
                bool draw_this = false; //boolean to determine if we draw this or continue to split
                if (hit) {

                    //************
                    //SHADOW RAY
                    //************
                    //determine shadow color if it's a shadow ray
                    if (ray->type == ray_type::shadow) {
                        //clean record list first
                        shadow_rec.shadow_frac.clear();
                        for (Light* light : lights) {
                            float sh_frac = 0;
                            //get hit position
                            vec3 ppos;
                            ray->get_point(rec.t, ppos);
                            //figure out the light-object intersection ray
                            vec3 loi_dir;
                            light->get_dir(loi_dir, ppos); //assumes result is normalized
                            //create a subset of the loi rays to generate soft shadows
                            Ray* loi_tr = new Ray(ppos, loi_dir, true, 0, ray->id, ray_type::shadow); //total, single shadow ray
                            std::vector<Ray*> loi_rays;
                            for (int split_n = 0; split_n < samples_per_ray; split_n++) {
                                Ray* r = new Ray(ppos, loi_dir, true, 0, ray->id, ray_type::shadow);
                                loi_rays.push_back(r);
                            }
                            //jitter split the single shatter ray by epsilon
                            loi_tr->split(loi_rays, samples_per_ray, ray_eps);
                            loi_rays.push_back(loi_tr);
                            //run hit test on each ray to figure out result
                            for (Ray* loi_r : loi_rays) {
                                hitrec sh_rec;
                                if (get_hit(aabb_tree, loi_r, epsilon, use_cam->far_clip, sh_rec)) sh_frac += 0;
                                else sh_frac += 1;
                                delete loi_r; //free heap memory
                            }
                            //average sampling result
                            sh_frac /= loi_rays.size();
                            //record into shadow record
                            shadow_rec.shadow_frac.push_back(sh_frac);
                            loi_rays.clear();
                        }
                        //record ray id into shadow record
                        shadow_rec.ray_id = ray->id;
                        draw_this = false;
                    }

                    //*********************
                    //RENDER RAY (SHADING)
                    //*********************
                    //determine mesh shade if it's a reflect ray
                    else {
                        for (Mesh* mesh : aabb_tree.meshes) {
                            //determine the mesh
                            if (!strcmp(mesh->id, rec.mesh_id)) {
                                //hitpoint
                                vec3 p;
                                ray->get_point(rec.t, p);

                                //don't generate subrays for reflectivity if the material doesn't need it
                                //it's not necessay to explicitly break the loop here, it will not elongate the loop so we will jump to next subpixel's shadow ray
                                if (!mesh->material->use_reflectivity) {
                                    vec3 contrib;
                                    mesh->material->apply_shade(contrib, p, use_cam->pos, rec.norm, lights, shadow_rec);
                                    //if the ray comes from a reflective ray, we need to apply contribution
                                    if (ray->contrib != 1) {
                                        vec3_mul_float(contrib, contrib, 1 - ray->total_previous_contrib);
                                    }
                                    vec3_add(ray->c_cache, ray->c_cache, contrib);
                                    vec3_deep_copy(c, ray->c_cache);
                                    vec3 weight_add;
                                    vec3_set_float(weight_add, ray->weight);
                                    vec3_add(weight_sum, weight_sum, weight_add);
                                    draw_this = true;
                                }

                                //********************************
                                // REFLECT RAYS
                                //********************************

                                //generate subrays for reflectivity rendering
                                else if (ray->depth > 0 && mesh->material->use_reflectivity && !mesh->material->use_refractivity) {

                                    //**********************************************
                                    //PROCESSING OF THIS RAY'S COLOR CONTRIBUTION
                                    //**********************************************

                                    //figure out the nonreflective color contribution of this hit ray (not the subray)
                                    vec3 c_self_contrib;
                                    vec3_zero(c_self_contrib);
                                    float prev_contrib = mesh->material->reflectivity;
                                    ray->contrib = 1 - prev_contrib;
                                    for (int contrib_n = ray->depth; contrib_n < max_ray_bounce; contrib_n++) {
                                        ray->contrib = prev_contrib * (1 - mesh->material->reflectivity);
                                        prev_contrib = prev_contrib - ray->contrib;
                                    }
                                    //figure out shadow contrib
                                    mesh->material->apply_shade(c_self_contrib, p, use_cam->pos, rec.norm, lights, shadow_rec);
                                    //apply self contrib factor
                                    vec3_mul_float(c_self_contrib, c_self_contrib, ray->contrib);
                                    //apply shadow factor
                                    //vec3_mul_float(c_self_contrib, c_self_contrib, ray->sh_cache);
                                    //concatenate with cache
                                    if (ray->depth == max_ray_bounce) vec3_deep_copy(ray->c_cache, c_self_contrib); //base case for the first hit
                                    else vec3_add(ray->c_cache, ray->c_cache, c_self_contrib);
                                    //record total previous contrib
                                    ray->total_previous_contrib += ray->contrib;


                                    //**********************************
                                    //SUB REFLECT RAY GENERATION
                                    //**********************************

                                    //create a single proper sub reflect ray instance
                                    Ray* rr = (Ray*)malloc(sizeof(Ray));
                                    rr->id = ray_id;
                                    rr->depth = ray->depth - 1;
                                    rr->type = ray_type::reflect;
                                    ray->reflect(rec.norm, rec.t, *rr);
                                    vec3_deep_copy(rr->c_cache, ray->c_cache);
                                    rr->sh_cache = ray->sh_cache;
                                    rr->contrib = ray->contrib;
                                    rr->total_previous_contrib = ray->total_previous_contrib;

                                    //create a monte carlo subset of this ray
                                    std::vector<Ray*> subrays;
                                    for (int split_n = 0; split_n < samples_per_ray; split_n++) {
                                        Ray* r = (Ray*)malloc(sizeof(Ray));
                                        r->id = ray_id;
                                        r->depth = rr->depth;
                                        r->type = ray_type::reflect;
                                        vec3_deep_copy(r->c_cache, ray->c_cache);
                                        r->sh_cache = rr->sh_cache;
                                        r->contrib = rr->contrib;
                                        r->total_previous_contrib = rr->total_previous_contrib;
                                        subrays.push_back(r);
                                    }

                                    //split and store into the subset vector
                                    rr->split(subrays, samples_per_ray, ray_eps);
                                    subrays.push_back(rr);

                                    //store into ray pool, this will elongate the ray processing loop until we get to bottom depth
                                    for (Ray* subray : subrays) {
                                        ray_pool->push(subray);
                                        //we create and store one more shadow ray here to calculate the subray's shadow factor on the run
                                        Ray* new_shadow_ray = (Ray*)malloc(sizeof(Ray));
                                        new_shadow_ray->id = ray_id;
                                        new_shadow_ray->type = ray_type::shadow;
                                        vec3_deep_copy(new_shadow_ray->e, rr->e);
                                        vec3_deep_copy(new_shadow_ray->d, rr->d);
                                        //we signal this kind of shadow ray to be depth -1 because we don't want to draw it
                                        new_shadow_ray->depth = -1;
                                        ray_pool->push(new_shadow_ray);
                                    }

                                    draw_this = false;
                                }

                                //if depth depleted on a render ray then just calculate shade (base case)
                                else if (ray->depth <= 0 && mesh->material->use_reflectivity && !mesh->material->use_refractivity) {
                                    //figure out the nonreflective color contribution of this hit ray (not the subray)
                                    vec3_deep_copy(c, ray->c_cache);
                                    vec3 weight_add;
                                    vec3_set_float(weight_add, ray->weight);
                                    vec3_add(weight_sum, weight_sum, weight_add);
                                    draw_this = true;
                                }

                                //*******************************
                                // REFRAC RAYS
                                //*******************************

                                //recursive case for a refractive material's shading rays
                                else if (ray->depth > 0 && mesh->material->use_refractivity) {
                                    RefracMat* mat = (RefracMat*)mesh->material;
                                    //figure out color cache for this ray, apply beer's law if it's in the material
                                    vec3 c_add;
                                    if (ray->refraci == mesh->material->refractivity) {
                                        float ar = mat->transp_c[0];
                                        float ag = mat->transp_c[1];
                                        float ab = mat->transp_c[2];
                                        c_add[0] = ar * exp(-ar * rec.t);
                                        c_add[1] = ag * exp(-ag * rec.t);
                                        c_add[2] = ab * exp(-ab * rec.t);
                                    }
                                    else {
                                        c_add[0] = 1;
                                        c_add[1] = 1;
                                        c_add[2] = 1;
                                    }

                                    //assign reflect ray
                                    Ray* reflect_ray = (Ray*)malloc(sizeof(Ray));
                                    reflect_ray->contrib = 1;
                                    reflect_ray->type = ray_type::refractive;
                                    if (ray->refraci == 1) {
                                        reflect_ray->depth = max_ray_bounce;
                                        reflect_ray->refraci = 1;
                                    }
                                    else {
                                        reflect_ray->depth = ray->depth - 1;
                                        reflect_ray->refraci = mesh->material->refractivity;
                                    }

                                    //figure out refrac ray
                                    Ray* refrac_ray = (Ray*)malloc(sizeof(Ray));
                                    refrac_ray->type = ray_type::refractive;
                                    refrac_ray->contrib = 1;
                                    //base and recurive cases for refrac ray depth
                                    if (ray->refraci == 1) refrac_ray->depth = max_refrac_bounce;
                                    else refrac_ray->depth = ray->depth - 1;
                                    refrac_ray->refraci = mesh->material->refractivity; //cache the refrac ray's refract index
                                    if (refrac_ray->refraci == ray->refraci) vec3_mul_float(rec.norm, rec.norm, -1); //reverse the norm if we are in the same refrac material for front and back face

                                    //figure out reflect ray
                                    ray->reflect(rec.norm, rec.t, *reflect_ray);
                                    float r_theta = ray->refrac(rec.norm, ray->refraci, mesh->material->refractivity, rec.t, *refrac_ray);

                                    //if total internal reflect just produce reflect ray
                                    if (r_theta <= 0) {
                                        free(refrac_ray); //we don't need to refrac in this case
                                        reflect_ray->weight = ray->weight; //set reflection to directly inherit parent weight
                                        vec3_fraction(reflect_ray->c_cache, ray->c_cache, c_add);
                                        ray_pool->push(reflect_ray);

                                        //we create and store one more shadow ray here to calculate the subray's shadow factor on the run
                                        Ray* new_shadow_ray = (Ray*)malloc(sizeof(Ray));
                                        new_shadow_ray->id = ray_id;
                                        new_shadow_ray->type = ray_type::shadow;
                                        vec3_deep_copy(new_shadow_ray->e, reflect_ray->e);
                                        vec3_deep_copy(new_shadow_ray->d, reflect_ray->d);
                                        //we signal this kind of shadow ray to be depth -1 because we don't want to draw it
                                        new_shadow_ray->depth = -1;
                                        ray_pool->push(new_shadow_ray);
                                    }
                                    //else send both refrac and reflect rays into stack
                                    else {
                                        //split the weight first
                                        reflect_ray->weight = r_theta * ray->weight;
                                        refrac_ray->weight = (1 - r_theta) * ray->weight;
                                        //record color cache
                                        vec3_fraction(reflect_ray->c_cache, ray->c_cache, c_add);
                                        vec3_mul_float(reflect_ray->c_cache, reflect_ray->c_cache, r_theta);
                                        ray_pool->push(reflect_ray);

                                        //we create and store one more shadow ray here to calculate the subray's shadow factor on the run
                                        Ray* new_shadow_ray = (Ray*)malloc(sizeof(Ray));
                                        new_shadow_ray->id = ray_id;
                                        new_shadow_ray->type = ray_type::shadow;
                                        vec3_deep_copy(new_shadow_ray->e, reflect_ray->e);
                                        vec3_deep_copy(new_shadow_ray->d, reflect_ray->d);
                                        //we signal this kind of shadow ray to be depth -1 because we don't want to draw it
                                        new_shadow_ray->depth = -1;
                                        ray_pool->push(new_shadow_ray);

                                        vec3_fraction(refrac_ray->c_cache, ray->c_cache, c_add);
                                        vec3_mul_float(refrac_ray->c_cache, refrac_ray->c_cache, 1 - r_theta);
                                        ray_pool->push(refrac_ray);

                                        //we create and store one more shadow ray here to calculate the subray's shadow factor on the run
                                        Ray* new_rshadow_ray = (Ray*)malloc(sizeof(Ray));
                                        new_rshadow_ray->id = ray_id;
                                        new_rshadow_ray->type = ray_type::shadow;
                                        vec3_deep_copy(new_rshadow_ray->e, refrac_ray->e);
                                        vec3_deep_copy(new_rshadow_ray->d, refrac_ray->d);
                                        //we signal this kind of shadow ray to be depth -1 because we don't want to draw it
                                        new_rshadow_ray->depth = -1;
                                        ray_pool->push(new_rshadow_ray);
                                    }

                                    draw_this = false;
                                }

                                //base case for a refractive material's shading ray
                                else if (ray->depth <= 0 && mesh->material->use_refractivity) {
                                    vec3_deep_copy(c, ray->c_cache);
                                    vec3 weight_add;
                                    vec3_set_float(weight_add, ray->weight);
                                    vec3_add(weight_sum, weight_sum, weight_add);
                                    draw_this = true;
                                }

                            }
                        }
                    }
                }

                //if not hit then figure out ray types and make decisions to return color (base case)
                else {
                    //case when we should draw a ray that's reflected but hit nothing
                    if (ray->type == ray_type::reflect && ray->depth < max_ray_bounce) {
                        vec3_deep_copy(c, ray->c_cache);
                        vec3 weight_add;
                        vec3_set_float(weight_add, ray->weight);
                        vec3_add(weight_sum, weight_sum, weight_add);
                        draw_this = true;
                    }
                    else if (ray->type == ray_type::refractive && ray->depth < max_refrac_bounce) {
                        vec3_deep_copy(c, ray->c_cache);
                        vec3 weight_add;
                        vec3_set_float(weight_add, ray->weight);
                        vec3_add(weight_sum, weight_sum, weight_add);
                        draw_this = true;
                    }
                }

                free(ray); //delete ray since we no longer needs it

                if (draw_this) {
                    color r;
                    vec3_to_color(r, c);
                    cs.push_back(r);
                }

                //**************************************
                //uncomment this section to do hit test
                //**************************************
                //while (ray_pool->size() != 0) {
                //    vec3 c;
                //    vec3 yellow = { 1.0f, 1.0f, 0.0f };
                //    vec3 dark = { 0.1f, 0.1f, 0.1f };
                //    Ray* ray = ray_pool->pop();
                //    bool hit = false;
                //    //get hit
                //    hitrec rec;
                //    rec.t = use_cam->far_clip;
                //    for (int i = 0; i < aabb_tree.meshes.size(); i++) {
                //        Mesh* mesh = aabb_tree.meshes[i];
                //        hitrec newrec;
                //        if (mesh->hit(*ray, use_cam->near_clip, use_cam->far_clip, newrec)) {
                //            hit = true;
                //            if ((newrec.t < rec.t)) {
                //                rec.t = newrec.t;
                //                strcpy(rec.mesh_id, newrec.mesh_id);
                //            }
                //        }
                //    }

                //    //show collision test result
                //    if (hit) {
                //        //determine hit mesh color
                //        for (int i = 0; i < aabb_tree.meshes.size(); i++) {
                //            Mesh* mesh = aabb_tree.meshes[i];
                //            if (!strcmp(mesh->id, rec.mesh_id)) {
                //                float ratio = (i + 1.0f) / aabb_tree.meshes.size();
                //                vec3 frac = { ratio, ratio * ratio, 1.0f };
                //                vec3_fraction(yellow, yellow, frac);
                //                break;
                //            }
                //        }
                //        vec3_deep_copy(c, yellow);
                //    }
                //    else vec3_deep_copy(c, dark);
                //    color r;
                //    vec3_to_color(r, c);
                //    cs.push_back(r);
            }

            rasterizer->setColor(i, j, cs, weight_sum);
        }
    }
    delete ray_pool;
}
