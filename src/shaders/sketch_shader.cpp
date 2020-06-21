#include "sketch_shader.h"

void draw_line(const vec2 line_dir, const float radius, const vec2 dot_dir, vec3 c){
    vec2 line_perp = {-1*line_dir[1], line_dir[0]};
    float dp = vec2_mul_inner(dot_dir, line_perp);
    if(dp >= 0 && fmod(dp, floor(radius/((1-c[0])*3*100))) < 0.1f){
        vec3_scale(c, c, 0.0f);
    }
    else if(dp < 0 && fmod(dp, floor(radius/((1-c[0])*3*100))) > -0.1f){
        vec3_scale(c, c, 0.0f);
    }
    else {
        c[0] = 1.0f;
        c[1] = 1.0f;
        c[2] = 1.0f;
    }
}

void gen_sketch_shade(const vec3 ca, const vec3 cr, const vec3 cl, const vec3 cp, vec3 l, const vec3 e, const vec3 norm, const float p,
const float x, const float y, const float radius, vec2 line_dir, vec3 c){
    gen_lambert_shade(ca, cr, cl, norm, l, c);
    gen_phong_shade(cl, cp, l, e, norm, p, c);

    //calculate a grayscale output (using luminosity method)
    rgb_to_grayscale(c, c);

    //draw silhouette
    if(norm[2]<=0.1f){
        vec3_scale(c, c, 0.0f);
        return;
    }

    //draw inner sketch with a tilted overline
    vec2 dot_dir = {x, y};
    vec2 new_line = {line_dir[0] + 0.1f, line_dir[1]};
    vec2_norm(new_line, new_line);
    vec3 cnew;
    vec3_scale(cnew, c, 1.0f);
    vec2_norm(new_line, new_line);
    draw_line(line_dir, radius, dot_dir, c);
    draw_line(new_line, radius, dot_dir, cnew);

    //composite
    if(cnew[0]==0.0f) vec3_scale(c, c, 0.0f);
}
