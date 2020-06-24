#include "fast_math.h"
#include <algorithm>

float fast_sqrt(float x)
 {
   unsigned int i = *(unsigned int*) &x; 
   // adjust bias
   i  += 127 << 23;
   // approximation of square root
   i >>= 1; 
   return *(float*) &i;
 } 

float fast_pow(float x, int y){
    float temp;
    if(y==0) return 1; //base case

    //recursion
    temp = fast_pow(x, y/2);
    if(y%2==0) return temp*temp;
    else return x*temp*temp;
}

void vec3_cull(vec3 r){
    r[0] = std::min(1.0f, r[0]);
    r[1] = std::min(1.0f, r[1]);
    r[2] = std::min(1.0f, r[2]);
}

void vec3_fraction(vec3 r, const vec3 a, const vec3 b){
    r[0] = a[0] * b[0];
    r[1] = a[1] * b[1];
    r[2] = a[2] * b[2];
}

void vec3_deep_copy(vec3 r, const vec3 a) {
    r[0] = a[0];
    r[1] = a[1];
    r[2] = a[2];
}

void rgb_to_grayscale(vec3 r, const vec3 rgb){
    for(int i=0; i < 3; i++){
        r[i] = 0.3f * rgb[0] + 0.59f * rgb[1] + 0.11f * rgb[2];
    }
}

float sqr(float x) { return x*x; }

void vec3_zero(vec3 r) {
    r[0] = 0.0f;
    r[1] = 0.0f;
    r[2] = 0.0f;
}

void aivec_to_vec3(vec3 r, const aiVector3D& v) {
    r[0] = v.x;
    r[1] = v.y;
    r[2] = v.z;
}

void aicolor_to_vec3(vec3 r, const aiColor3D& c) {
    r[0] = c.r;
    r[1] = c.g;
    r[2] = c.b;
}
