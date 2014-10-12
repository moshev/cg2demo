#if !defined(CG2DEMO_SCENEDSL_H)
#define CG2DEMO_SCENEDSL_H

#include <stdint.h>
#include <math.h>

#include "scene.h"

/*
prefix meaning:
df - distance function
gf - generic function
vf - vector function
<empty> - any
*/

#define SC_VEC3(gfx, gfy, gfz) VF_VECTOR, gfx, gfy, gfz
#define SC_CUBE(v3centre, gfsz) DF_CUBE, v3centre, gfsz
#define SC_CUBE3(v3centre, v3sz) DF_CUBE3, v3centre, v3sz
#define SC_SPHERE(v3centre, gfradius) DF_SPHERE, v3centre, gfradius
#define SC_TORUS(v3centre, v3normal, gfrc, gfrt) DF_TORUS, v3centre, v3normal, gfrc, gfrt
#define SC_CYLINDER_CAP(v3A, v3B, gfradius) DF_CYLINDER_CAP, v3A, v3B, gfradius
#define SC_PLANE(v3P, v3N) DF_PLANE, v3P, v3N
#define SC_MIX(A, B, C) DF_MIX, A, B, C
#define SC_MIN(A, B) DF_MIN, A, B
#define SC_MAX(A, B) DF_MAX, A, B
#define SC_FIXED(f) GF_NUMBER, ((uint8_t)f), ((uint8_t)(f * 256))
#define SC_TIME(n) GF_TIME, n
#define SC_TIME2(n) GF_TIME2, n
#define SC_CLAMP(gfarg, gfmin, gfmax) GF_CLAMP, gfarg, gfmin, gfmax
#define SC_SMOOTH(gfmin, gfmax, gfarg) GF_SMOOTH, gfmin, gfmax, gfarg


#endif
