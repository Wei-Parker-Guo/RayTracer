#ifndef RENDER_THREAD
#define RENDER_THREAD

#include "raypool.h"
#include "rasterizer.h"
#include "cameras.h"
#include "geometry.h"
#include "AABBTree.h"
#include <GLFW/glfw3.h>

struct render_ray_rec {
	int ray_id;
	vec3 c;
};

//function to determine the hit rec of a ray
bool get_hit(const AABBTree& aabb_tree, const Ray* ray, const float t0, const float t1, hitrec& rec);

class RenderThread {
	public:
		//thread entry point, renders pixels from (startx, starty) to (endx, endy) with endpoint exclusive
		void operator()(Rasterizer* rasterizer, AABBTree& aabb_tree, Camera* use_cam, std::vector<Light*> lights,
			const int startX, const int startY, const int endX, const int endY,
			const int ray_pool_page_size, const float set_hfov, const int samples_per_pixel, const int samples_per_ray, const int max_ray_bounce, const float epsilon, const float ray_eps,
			const float max_refrec_bounce);
};

#endif
