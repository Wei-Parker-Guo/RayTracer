#ifndef CAMERAS
#define CAMERAS

#include "fast_math.h"
#include "assimp/camera.h"

//class representing a camera object, right handed system with world coordinates
class Camera {
	public:
		//local coordinates
		vec3 pos;
		vec3 up;
		vec3 lookat;
		vec3 side;
		float hfov; //horizontal fov
		float near_clip;
		float far_clip;

		//constructors
		Camera(const aiCamera* cam, const aiMatrix4x4& gtrans);
};

#endif