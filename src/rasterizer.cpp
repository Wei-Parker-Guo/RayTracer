#include "rasterizer.h"

void color_to_vec3(vec3 r, const color& c) {
	r[0] = c.r;
	r[1] = c.g;
	r[2] = c.b;
}

void vec3_to_color(color& r, const vec3 c) {
	r.r = c[0];
	r.g = c[1];
	r.b = c[2];
}

Rasterizer::Rasterizer(const int width, const int height) {
	//setup asperct ratio
	this->global_width = width;
	this->global_height = height;
	//push black pixels
	for (int i = 0; i < width; i++) {
		colorseq cs;
		for (int j = 0; j < height; j++) {
			color c;
			cs.push_back(c);
		}
		this->buf.push_back(cs);
	}
}

int Rasterizer::getHeight() {
	return this->global_height;
}

int Rasterizer::getWidth() {
	return this->global_width;
}

void Rasterizer::getColor(const int x, const int y, vec3 c) {
	color_to_vec3(c, this->buf[x][y]);
}

void Rasterizer::resize(const float width, const float height) {
	//figure out if we are bigger or smaller and adjust buf accordingly, saving as many data as possible

	//equal situation, short chain quit
	if (this->global_height == height && this->global_width == width) return;

	//bigger/smaller situation
	if (this->global_width < width) {
		for (int i = this->global_width; i < width; i++) {
			//push black pixels to end of each row
			colorseq cs;
			for (int j = 0; j < this->global_height; j++) {
				color c;
				cs.push_back(c);
			}
			this->buf.push_back(cs);
		}
		this->global_width = width;
	}
	else if (this->global_width > width) {
		for (int i = width; i < this->global_width; i++) {
			this->buf.pop_back();
		}
		this->global_width = width;
	}

	if (this->global_height < height) {
		for (int i = 0; i < this->global_width; i++) {
			for (int j = this->global_height; j < height; j++) {
				color c;
				this->buf[i].push_back(c);
			}
		}
		this->global_height = height;
	}
	else if (this->global_height > height) {
		for (int i = 0; i < this->global_width; i++) {
			for (int j = height; j < this->global_height; j++) {
				this->buf[i].pop_back();
			}
		}
		this->global_height = height;
	}
	
}

void Rasterizer::setColor(const int x, const int y, const colorseq& cseq) {
	//average the colors
	vec3 c;
	vec3_zero(c);
	for (int i = 0; i < cseq.size(); i++) {
		vec3 c_n;
		color_to_vec3(c_n, cseq.at(i));
		vec3_add(c, c, c_n);
	}
	vec3_scale(c, c, 1.0f / (float) cseq.size());

	//record
	color r;
	vec3_to_color(r, c);
	this->buf[x][y] = r;
}
