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
    const int ray_pool_page_size, const float set_hfov, const int samples_per_pixel, const int samples_per_ray, const int max_ray_bounce, const float epsilon, const float ray_eps) {

    //initialize a ray allocator
    RayPool ray_pool = RayPool(ray_pool_page_size);

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
                    //add to pool
                    ray_pool.push(new_render_ray);

                    //construct shadow ray
                    Ray* new_shadow_ray = (Ray*)malloc(sizeof(Ray));
                    new_shadow_ray->id = ray_id;
                    new_shadow_ray->type = ray_type::shadow;
                    vec3_deep_copy(new_shadow_ray->e, use_cam->pos);
                    vec3_deep_copy(new_shadow_ray->d, ray_dir);
                    //add to pool
                    ray_pool.push(new_shadow_ray);

                    //construct layered depth rays
                    for (int d = max_ray_bounce - 1; d >= 0; d--) {

                    }

                    //update ray id
                    ray_id += 1;
                }
            }

            //**************************************
            //Calculation of the ray return color
            //**************************************

            std::vector<shadow_ray_rec*> shadow_fracs;

            while (ray_pool.size() != 0) {
                Ray* ray = ray_pool.pop();
                hitrec rec;
                bool hit = get_hit(aabb_tree, ray, use_cam->near_clip, use_cam->far_clip, rec);

                //handle the info and calculate color
                vec3 c;
                vec3_zero(c);
                if (hit) {
                    //determine shadow color if it's a shadow ray
                    if (ray->type == ray_type::shadow) {
                        float total_sh_frac = 0; //result
                        for (Light* light : lights) {
                            float sh_frac = 0;
                            //get hit position
                            vec3 ppos;
                            ray->get_point(rec.t, ppos);
                            //figure out the light-object intersection ray
                            vec3 loi_dir;
                            light->get_dir(loi_dir, ppos); //assumes result is normalized
                            vec3 loi_pos;
                            vec3_mul_float(loi_pos, loi_dir, epsilon);
                            vec3_add(loi_pos, loi_pos, ppos);
                            //create a subset of the loi rays to generate soft shadows
                            Ray* loi_tr = new Ray(loi_pos, loi_dir, true, 0, ray->id, ray_type::shadow); //total, single shadow ray
                            std::vector<Ray*> loi_rays;
                            for (int split_n = 0; split_n < samples_per_ray; split_n++) {
                                Ray* r = new Ray(loi_pos, loi_dir, true, 0, ray->id, ray_type::shadow);
                                loi_rays.push_back(r);
                            }
                            //jitter split the single shatter ray by epsilon
                            loi_tr->split(loi_rays, samples_per_ray, ray_eps);
                            //run hit test on each ray to figure out result
                            for (Ray* loi_r : loi_rays) {
                                hitrec sh_rec;
                                if (get_hit(aabb_tree, loi_r, epsilon, use_cam->far_clip, sh_rec)) sh_frac += 0;
                                else sh_frac += 1;
                            }
                            //average sampling result
                            sh_frac /= loi_rays.size();
                            total_sh_frac += sh_frac;
                        }
                        //average light_result
                        total_sh_frac /= lights.size();
                        shadow_ray_rec sh_r_rec;
                        sh_r_rec.shadow_frac = total_sh_frac;
                        sh_r_rec.ray_id = ray->id;
                        shadow_fracs.push_back(&sh_r_rec);
                    }

                    //determine mesh shade if it's a reflect ray
                    else {
                        for (int i = 0; i < aabb_tree.meshes.size(); i++) {
                            Mesh* mesh = aabb_tree.meshes[i];
                            //determine the mesh
                            if (!strcmp(mesh->id, rec.mesh_id)) {
                                vec3 p;
                                ray->get_point(rec.t, p);
                                mesh->material->apply_shade(c, p, rec.norm, lights);
                                break;
                            }
                        }
                        //apply shadow fraction
                        for (shadow_ray_rec* frac : shadow_fracs) {
                            if (frac->ray_id == ray->id) {
                                vec3_mul_float(c, c, frac->shadow_frac);
                                break;
                            }
                        }
                    }
                }
                else vec3_zero(c);
                free(ray); //delete ray since we no longer needs it
                color r;
                vec3_to_color(r, c);
                cs.push_back(r);

            //**************************************
            //uncomment this section to do hit test
            //**************************************
            //while (ray_pool.size() != 0) {
            //    vec3 c;
            //    vec3 yellow = { 1.0f, 1.0f, 0.0f };
            //    vec3 dark = { 0.1f, 0.1f, 0.1f };
            //    Ray* ray = ray_pool.pop();
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
                //color r;
                //vec3_to_color(r, c);
                //cs.push_back(r);
            }

            rasterizer->setColor(i, j, cs);
        }
    }
}
