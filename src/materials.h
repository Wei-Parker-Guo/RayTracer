#ifndef MATERIALS_H
#define MATERIALS_H

#include <vector>
#include <assimp/material.h>
#include "fast_math.h"
#include "lights.h"

enum class mat_texture_type {
	base_color,
	specular,
	metallic,
	height,
	bump,
	roughness
};

struct Texture {
	unsigned int id;
	mat_texture_type type;
};

//the virtual class representing all materials
class Material {
	public:
		bool use_reflectivity = false;
		float reflectivity = 0;
		//method to apply the shade on an input color to retrieve the shaded one, with lights and triangle info given
		virtual void apply_shade(vec3 r, const vec3 p, const vec3 e, const vec3 norm, std::vector<Light*> lights);
};

//a basic lambert material
class LambertMat : public Material {
	public:
		vec3 base_c;
		vec3 ambient_c;
		//std::vector<Texture*> texes;

		//constructors
		LambertMat(aiMaterial* mat);
		LambertMat(const vec3 base_color, const vec3 ambient_color);

		//methods
		void apply_shade(vec3 r, const vec3 p, const vec3 e, const vec3 norm, std::vector<Light*> lights) override;
};

//basic, ansiotropic Phong Material that could also be used to render isotropic materials
class PhongMat : public Material {
	public:
		vec3 base_c;
		vec3 ambient_c;
		vec3 spec_c;
		float shininess;

		//constructors
		PhongMat(aiMaterial* mat);

		//methods
		void apply_shade(vec3 r, const vec3 p, const vec3 e, const vec3 norm, std::vector<Light*> lights) override;
};

#endif
