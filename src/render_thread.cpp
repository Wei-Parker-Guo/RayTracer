#include "render_thread.h"

const float PI = 3.1415926;

void RenderThread::operator()(Rasterizer* rasterizer, std::vector<Mesh*>& meshes, Camera* use_cam,
    const int startX, const int startY, const int endX, const int endY, 
    const int ray_pool_page_size, const float set_hfov, const int samples_per_pixel, const int max_ray_bounce) {

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

                    //construct ray
                    Ray* new_render_ray = (Ray*)malloc(sizeof(Ray));
                    vec3_deep_copy(new_render_ray->e, use_cam->pos);
                    vec3_deep_copy(new_render_ray->d, ray_dir);
                    new_render_ray->depth = max_ray_bounce;
                    //add to pool
                    ray_pool.push(new_render_ray);
                }
            }

            //hit test
            while (ray_pool.size() != 0) {
                vec3 c;
                vec3 yellow = { 1.0f, 1.0f, 0.0f };
                vec3 dark = { 0.1f, 0.1f, 0.1f };
                Ray* ray = ray_pool.pop();
                bool hit = false;
                for (int i = 0; i < meshes.size(); i++) {
                    Mesh* mesh = meshes[i];
                    hitrec rec;
                    hit = mesh->hit(*ray, d, use_cam->far_clip, rec);
                    if (hit) break;
                }
                free(ray);
                if (hit) {
                    vec3_deep_copy(c, yellow);
                }
                else vec3_deep_copy(c, dark);
                color r;
                vec3_to_color(r, c);
                cs.push_back(r);
            }

            rasterizer->setColor(i, j, cs);
        }
    }
}
