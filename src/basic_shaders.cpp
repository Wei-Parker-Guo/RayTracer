#include "basic_shaders.h"

/* generate lambertian shade with another light source
 * equation: c = cr(ca + clmax(0, n.l))
 */
void gen_lambert_shade(const vec3 ca, const vec3 cr, const vec3 cl, const vec3 n, vec3 l, vec3 c){
    //normalize light vector
    vec3_norm(l, l);

    //reflectance
    float r = std::max(0.0f, vec3_mul_inner(n,l));
    vec3_scale(c, cl, r);
    vec3_add(c, c, ca);
    //apply reflect fraction
    vec3_fraction(c, cr, c);
}

/* generate phong shade based on a precalculated lambertian shade
equation: c = clambert + clcpmax(e.r)^p */
void gen_phong_shade(const vec3 cl, const vec3 cp, const vec3 l, const vec3 e, const vec3 n, const int p, vec3 clambert){
    //calculate r
    vec3 r;
    vec3_scale( r, n, 2.0f*vec3_mul_inner(l,n) );
    vec3_sub(r, r, l);
    vec3_norm(r,r);

    //calculate shade
    vec3 cphong;
    vec3_scale(cphong, cp, fast_pow(std::max(0.0f,vec3_mul_inner(e,r)),p));
    vec3_fraction(cphong, cl, cphong);
    vec3_add(clambert, clambert, cphong);
    vec3_cull(clambert);
}

/* generate anisotropic phong shade by dividing the normal into uv directions
and calculating specular based on the Ward Anisotropic Distribution Model (a BRDF), finally adding the lambertian base shade.
Notice this can't use our fast_pow since the exponential is float.
The formula is referenced from this url:
https://en.wikibooks.org/wiki/GLSL_Programming/Unity/Brushed_Metal */
void gen_WARD_anisotropic_phong_shade(const vec3 cl, const vec3 cp, const vec3 l, const vec3 e, const vec3 n, const float pu, const float pv, const float y, vec3 clambert){
    //calculate h
    vec3 h;
    vec3_add(h, e, l);
    vec3_norm(h,h);

    //calculate u and v unit vector
    vec3 v;
    vec3 y_v = {0.0f, y, 0.0f};
    vec3_scale(v, n, vec3_mul_inner(n,y_v));
    vec3_sub(v, y_v, v);
    vec3_norm(v,v);
    vec3 u;
    vec3_mul_cross(u, v, n);
    vec3_norm(u,u);
    
    //calculate kspecular
    vec3 kspec;
    vec3_scale(kspec, cp, sqrt( std::max(0.0f, vec3_mul_inner(n,l) / vec3_mul_inner(n,l)*vec3_mul_inner(n,e)) )); //first term of the multiplication
    vec3_scale(kspec, kspec, 1.0f / (4.0f*PI*pu*pv) ); //second term
    vec3_scale(kspec, kspec, exp( -2.0f * (sqr(vec3_mul_inner(h,u)/pu) + sqr(vec3_mul_inner(h,v)/pv)) / (1.0f+vec3_mul_inner(h,n)) ) ); //thrid term
    vec3_fraction(kspec, kspec, cl); //control one more time by light color

    //add specular to lambertian shade
    vec3_add(clambert, clambert, kspec);
    vec3_cull(clambert);
}
