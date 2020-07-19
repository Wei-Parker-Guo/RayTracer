#ifndef RASTERIZER
#define RASTERIZER

#include <vector>
#include "fast_math.h"

struct color {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
};

typedef std::vector<color> colorseq;
typedef std::vector<colorseq> imagebuffer;

//a simple function to copy a color struct's values into a vec3 vector
void color_to_vec3(vec3 r, const color& c);

//a simple function to copy a vec3 vector into a color struct's values
void vec3_to_color(color& r, const vec3 c);

//class representing a rasterizer, stores one "image" and can update them using antialiazing
class Rasterizer {

	private:
		//aspect ratio
		int global_width;
		int global_height;
		//buffer as a 2d vector
		imagebuffer buf;

	public:
		//constructor of an empty buffed rasterizer with the aspect ratio given
		Rasterizer(const int width, const int height);
		//accessors
		int getHeight();
		int getWidth();
		void getColor(const int x, const int y, vec3 c); //get the final color result at the specific buffer coordinate
		//mutators
		void resize(const float width, const float height); //this resizes the buf, will reduce/add empty black pixels as needed
		//utility methods
		void setColor(const int x, const int y, const colorseq& cseq, const vec3 weight_sum); //this accepts a color sequence of n vec3 colors, average the result and set into the specified pixel
};


#endif
