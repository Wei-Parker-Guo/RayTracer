#include "layered_toon_shader.h"

// A simple function that layers a color with ratio given
void vec3_layer(vec3 r, const vec3 maxr, const float max_n, const float n){
    //set to lowest layer color
    r[0] = 0.0f;
    r[1] = 0.0f;
    r[2] = 0.0f;

    //a color add for each layer
    vec3 cadd;
    vec3_scale(cadd, maxr, 1/max_n);

    //cal current layer color
    for(float i=0.0f; i<n; i+=1.0f){
        vec3_add(r, r, cadd);
    }
}

/* generate cartoon like artistic shade based linear blend of a warm color and a cold color
it also takes a phong highlight base and linearly layered the specular to put on top of basic shade
with respect to light source. cc2 is warm cc1 is cold. It also takes a layer param for rendering layered speculars. */
void gen_toon_shade(const vec3 cc1, const vec3 cc2, const vec3 l, const vec3 cl, const vec3 cp, const float toonl, const vec3 e, const vec3 norm, vec3 cphong){
    //get a normalized light
    vec3 nl;
    vec3_norm(nl, l);

    //draw silhouette
    if(vec3_mul_inner(e,norm)<=0.15f){
        vec3_sub(cphong, cphong, cphong);
        return;
    }

    //treat the highlight as linearly distributed (layers)
    float light_value = vec3_len(cphong);
    vec3 max_reflect;
    vec3_fraction(max_reflect, cl, cp);
    float max_light = vec3_len(max_reflect);
    for(float i=0.0f; i<toonl; i+=1.0f){
        if ( light_value < (max_light/toonl*(i+1.0f)) && light_value > (max_light/toonl*i) ) {
            vec3_layer(cphong, cl, toonl, i+1.0f);
            break;
        }
    }

    //calculate a linear blend
    float kw = (1 + vec3_mul_inner(norm, nl))/2;
    vec3 c_warm;
    vec3 c_cold;
    vec3_scale(c_warm, cc2, kw);
    vec3_scale(c_cold, cc1, 1.0f - kw);
    vec3_add(cphong, cphong, c_cold);
    vec3_add(cphong, cphong, c_warm);
}
