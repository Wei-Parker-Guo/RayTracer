#ifndef MATERIALS
#define MATERIALS

#include <vector>
#include "fast_math.h"
#include "geometry.h"
#include "lights.h"

enum class texture_type {
	base_color,
	specular,
	metallic,
	height,
	bump,
	roughness
};

struct Texture {
	unsigned int id;
	texture_type type;
};

//the virtual class representing all materials
class Material {
	public:
		//list of communal attributes
		vec3 base_c;
		vec3 ambient_c;
		vec3 reflect_c;
		std::vector<Texture*> texes;

		//virtual methods to override

		//method to apply the shade on an input color to retrieve the shaded one, with light and triangle info given
		virtual void apply_shade(vec3 r, const vec3 p, const Triangle& t, Light& light);
};

//a basic lambert material
class LambertMat : Material {
	public:
		//constructor
		LambertMat(const vec3 base_c, const vec3 ambient_c, const vec3 reflect_c);

		//methods
		void apply_shade(vec3 r, const vec3 p, const Triangle& t, Light& light);
};

#endif
