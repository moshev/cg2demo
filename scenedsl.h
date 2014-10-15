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

template <int x>
struct _hi {
    static const uint8_t v = (uint8_t)((x >> 8) & 0xFF);
};

template <int x>
struct _lo {
    static const uint8_t v = (uint8_t)((x) & 0xFF);
};

#define SCENESPEC(tx, ty, tz, duration, dfscene)\
    _hi<(int)((tx) * 256)>::v, _lo<(int)((tx) * 256)>::v,\
    _hi<(int)((ty) * 256)>::v, _lo<(int)((ty) * 256)>::v,\
    _hi<(int)((tz) * 256)>::v, _lo<(int)((tz) * 256)>::v,\
    _hi<(int)(duration)>::v, _lo<(int)(duration)>::v,\
    dfscene

#define SC_VEC3(gfx, gfy, gfz) VF_VECTOR, gfx, gfy, gfz
#define SC_TILED(vtile, dfobj) DF_TILED, vtile, dfobj
#define SC_CUBE(v3centre, gfsz) DF_CUBE, v3centre, gfsz
#define SC_CUBE3(v3centre, v3sz) DF_CUBE3, v3centre, v3sz
#define SC_SPHERE(v3centre, gfradius) DF_SPHERE, v3centre, gfradius
#define SC_TORUS(v3centre, v3normal, gfrc, gfrt) DF_TORUS, v3centre, v3normal, gfrc, gfrt
#define SC_CYLINDER_CAP(v3A, v3B, gfradius) DF_CYLINDER_CAP, v3A, v3B, gfradius
#define SC_PLANE(v3P, v3N) DF_PLANE, v3P, v3N
#define SC_MIX(A, B, C) DF_MIX, A, B, C
#define SC_MIN(A, B) DF_MIN, A, B
#define SC_MAX(A, B) DF_MAX, A, B
#define SC_FIXED(f) GF_NUMBER, _hi<(int)((f) * 256)>::v, _lo<(int)((f) * 256)>::v
#define SC_TIME(n) GF_TIME, n
#define SC_TIME2(n) GF_TIME2, n
#define SC_CLAMP(gfarg, gfmin, gfmax) GF_CLAMP, gfarg, gfmin, gfmax
#define SC_SMOOTH(gfmin, gfmax, gfarg) GF_SMOOTH, gfmin, gfmax, gfarg
#define SC_PX GF_PDOTX
#define SC_PY GF_PDOTY
#define SC_PZ GF_PDOTZ


#endif
