#include "translucent_shader.h"

void gen_translucent_shade(const vec3 ca, const vec3 cr, const vec3 cl, const vec3 cp, vec3 l, const vec3 e, const vec3 norm, const float p,
float x, float y, const float ds, const float dd, const float depth, vec3 c){
    //cal diffuse and spec for inner and outer layer and blend
    gen_lambert_shade(ca, cr, cl, norm, l, c);
    gen_phong_shade(cl, cp, l, e, norm, p, c);
    vec3_scale(c, c, dd); //scale outer color by dimming with depth decay factor
    //apply BTDF to simulate a sum of subsurface source (a forward dipole)
    vec3 ci_total = {0.0f, 0.0f, 0.0f};

    vec3 vbase = {norm[0] - ds, norm[1] - depth, norm[2]}; //pick a subsurface source to start with, set our microfacet dipole to be ds unit away
    vec3_norm(vbase, vbase);
    //approximate an intergral of these subsurface sources
    for(float i = 0.0f; i < 2*PI; i+=1.0f){
        //find rotated vector around the normal by Rodrigue's rotation formula
        vec3 vrot;
        //first term
        vec3_mul_cross(vrot, norm, vbase);
        vec3_scale(vrot, vrot, sin(i));
        //second term
        vec3 vsec;
        vec3_scale(vsec, vbase, cos(i));
        //third term
        vec3 vthird;
        vec3_scale(vthird, norm, vec3_mul_inner(norm, vbase) * (1 - cos(i)) );
        //add and find rotated vec
        vec3_add(vrot, vrot, vsec);
        vec3_add(vrot, vrot, vthird);

        //calculate BTDF for this derivative
        vec3 ci;
        gen_lambert_shade(ca, cr, cl, norm, vrot, ci);
        gen_phong_shade(cl, cp, vrot, e, norm, p, ci);
        vec3_add(ci_total, ci_total, ci);
    }

    vec3_scale(ci_total, ci_total, 1 - dd); //preserve energy by culling with depth decay

    //composite
    vec3_add(c, c, ci_total);
}