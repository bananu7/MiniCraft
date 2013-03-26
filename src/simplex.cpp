#include "simplex.h"
#include <cmath>

float grad[12][3] = {
    {1.0,1.0,0.0},{-1.0,1.0,0.0},{1.0,-1.0,0.0},{-1.0,-1.0,0.0},
    {1.0,0.0,1.0},{-1.0,0.0,1.0},{1.0,0.0,-1.0},{-1.0,0.0,-1.0},
    {0.0,1.0,1.0},{0.0,-1.0,1.0},{0.0,1.0,-1.0},{0.0,-1.0,-1.0}
};

int perm[512] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};

float dot(float x, float y, float* g) {
    return x*g[0] + y*g[1];
}
float dot(float x, float y, float z, float* g) {
    return x*g[0] + y*g[1] + z*g[2];
}

float noise3d(float xin, float yin, float zin) {
    float F3, G3, t, X0, Y0, Z0, x0, y0, z0, s, x1, y1, z1, x2, y2, z2, x3, y3, z3, t0, t1, t2, t3, n0, n1, n2, n3;
    int i, j, k, ii, jj, kk, i1, j1, k1, i2, j2, k2, gi0, gi1, gi2, gi3;
    
    F3 = 1.0/3.0;
    s = (xin+yin+zin)*F3;
    i = xin+s;
    j = yin+s;
    k = zin+s;
    G3 = 1.0/6.0;
    t = (i+j+k)*G3;
    X0 = i-t;
    Y0 = j-t;
    Z0 = k-t;
    x0 = xin-X0;
    y0 = yin-Y0;
    z0 = zin-Z0;
    
    if(x0 >= y0){
        if(y0 >= z0){
            i1=1; j1=0; k1=0; i2=1; j2=1; k2=0;
        }
        else if(x0 >= z0){
             i1=1; j1=0; k1=0; i2=1; j2=0; k2=1;
        }
        else{
            i1=0; j1=0; k1=1; i2=1; j2=0; k2=1;
        }
    }
    else{
        if(y0 < z0){
            i1=0; j1=0; k1=1; i2=0; j2=1; k2=1;
        }
        else if(x0 < z0){ 
            i1=0; j1=1; k1=0; i2=0; j2=1; k2=1;
        }
        else{
            i1=0; j1=1; k1=0; i2=1; j2=1; k2=0;
        }
    }

    x1 = x0 - i1 + G3;
    y1 = y0 - j1 + G3;
    z1 = z0 - k1 + G3;
    x2 = x0 - i2 + 2.0*G3;
    y2 = y0 - j2 + 2.0*G3;
    z2 = z0 - k2 + 2.0*G3;
    x3 = x0 - 1.0 + 3.0*G3;
    y3 = y0 - 1.0 + 3.0*G3;
    z3 = z0 - 1.0 + 3.0*G3;

    ii = i & 255;
    jj = j & 255;
    kk = k & 255;
    
    gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
    gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
    gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
    gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;
    
    t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if(t0<0){
         n0 = 0.0;
    }
    else{
        t0 *= t0;
        n0 = t0 * t0 * dot(x0, y0, z0, grad[gi0]);
    }

    t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if(t1<0){
         n1 = 0.0;
    }
    else{
        t1 *= t1;
        n1 = t1 * t1 * dot(x1, y1, z1, grad[gi1]);
    }

    t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if(t2<0){
         n2 = 0.0;
    }
    else{
        t2 *= t2;
        n2 = t2 * t2 * dot(x2, y2, z2, grad[gi2]);
    }

    t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if(t3<0){
         n3 = 0.0;
    }
    else{
        t3 *= t3;
        n3 = t3 * t3 * dot(x3, y3, z3, grad[gi3]);
    }

    return 16.0*(n0 + n1 + n2 + n3)+1.0;
}

 // 2D simplex noise
double noise2d(double xin, double yin) {
    double n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in
    const double F2 = 0.5*(sqrt(3.0)-1.0);
    double s = (xin+yin)*F2; // Hairy factor for 2D
    int i = floor(xin+s);
    int j = floor(yin+s);
    const double G2 = (3.0-sqrt(3.0))/6.0;
    double t = (i+j)*G2;
    double X0 = i-t; // Unskew the cell origin back to (x,y) space
    double Y0 = j-t;
    double x0 = xin-X0; // The x,y distances from the cell origin
    double y0 = yin-Y0;
    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)
    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    double y2 = y0 - 1.0 + 2.0 * G2;
    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = perm[ii+perm[jj]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1]] % 12;
    int gi2 = perm[ii+1+perm[jj+1]] % 12;
    // Calculate the contribution from the three corners
    double t0 = 0.5 - x0*x0-y0*y0;
    if(t0<0)
        n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(x0, y0, grad[gi0]); // (x,y) of grad3 used for 2D gradient
    }
    double t1 = 0.5 - x1*x1-y1*y1;
    if(t1<0)
        n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(x1, y1, grad[gi1]);
    }
    double t2 = 0.5 - x2*x2-y2*y2;
    if(t2<0) 
        n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(x2, y2, grad[gi2]);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0 * (n0 + n1 + n2);
 }
 
float simplex_noise(int octaves, float x, float y, float z){
    float value = 0.0;
    int i;
    for(i=0; i<octaves; i++){
        value += noise3d(
            x*pow(2, i),
            y*pow(2, i),
            z*pow(2, i)
        );
    }
    return value;
}

float simplex_noise(int octaves, float x, float y){
    float value = 0.0;
    int i;
    for(i=0; i<octaves; i++){
        value += noise2d(
            x*pow(2, i),
            y*pow(2, i)
        );
    }
    return value;
}

