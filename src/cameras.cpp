#include "cameras.h"

Camera::Camera(const aiCamera* cam, const aiMatrix4x4& gtrans) {
	//local coordinates
	aivec_to_vec3(this->pos, cam->mPosition);
	aivec_to_vec3(this->up, cam->mUp);
	aivec_to_vec3(this->lookat, cam->mLookAt);
	vec3_norm(this->up, this->up);
	vec3_scale(this->lookat, this->lookat, -1.0f);
	vec3_norm(this->lookat, this->lookat);
	vec3_mul_cross(this->side, this->up, this->lookat);
	vec3_norm(this->side, this->side);

	//other needed propertities
	this->hfov = cam->mHorizontalFOV;
	this->near_clip = cam->mClipPlaneNear;
	this->far_clip = cam->mClipPlaneFar;
}
