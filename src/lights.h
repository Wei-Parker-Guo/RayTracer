#ifndef LIGHTS
#define LIGHTS

#include "fast_math.h"

//class representing a general light object
class Light {
	public:
		//method to figure out a light's direction and color based on a point given
		virtual void get_dir_c(vec3 r, vec3 c, const vec3 ppos);
};

class DirectLight : public Light {
	private:
		vec3 color;
		vec3 direction;
	public:
		void get_dir_c(vec3 r, vec3 c, const vec3 ppos);
};

#endif
