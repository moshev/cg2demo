#define __USE_MISC 1
#include <math.h>

#include "math3d_t.h"

static inline vec2 mkv2(float v0, float v1) {
    vec2 r = { { v0, v1 } };
    return r;
}

static inline vec3 mkv3(float v0, float v1, float v2) {
    vec3 r = { { v0, v1, v2 } };
    return r;
}

static inline vec4 mkv4(float v0, float v1, float v2, float v3) {
    vec4 r = { { v0, v1, v2, v3 } };
    return r;
}

static inline vec3 exv2v3(vec2 v) {
    return mkv3(v.v[0], v.v[1], 0);
}

static inline vec4 exv2v4(vec2 v) {
    return mkv4(v.v[0], v.v[1], 0, 0);
}

static inline vec4 exv3v4(vec3 v) {
    return mkv4(v.v[0], v.v[1], v.v[2], 0);
}

static inline mat4 exm3m4(mat3 m) {
    mat4 r;
    r.c[0] = exv3v4(m.c[0]);
    r.c[1] = exv3v4(m.c[1]);
    r.c[2] = exv3v4(m.c[2]);
    r.c[3] = mkv4(0, 0, 0, 1);
    return r;
}

static inline mat3 mkm3(vec3 c0, vec3 c1, vec3 c2) {
    /* column-major, so the actual matrix
     * is the transpose of what you're looking at*/
    mat3 r;
    r.c[0] = c0;
    r.c[1] = c1;
    r.c[2] = c2;
    return r;
}

static inline mat3 mkm3identity() {
    return mkm3(
        mkv3(1, 0, 0),
        mkv3(0, 1, 0),
        mkv3(0, 0, 1));
}

static inline mat4 mkm4(vec4 c0, vec4 c1, vec4 c2, vec4 c3) {
    /* column-major, so the actual matrix
     * is the transpose of what you're looking at*/
    mat4 r;
    r.c[0] = c0;
    r.c[1] = c1;
    r.c[2] = c2;
    r.c[3] = c3;
    return r;
}

static inline mat4 mkm4identity() {
    return mkm4(
        mkv4(1, 0, 0, 0),
        mkv4(0, 1, 0, 0),
        mkv4(0, 0, 1, 0),
        mkv4(0, 0, 0, 1));
}

static const float TAU = 6.283185307179586476925286766559f;

static inline vec2 negv2(vec2 a) {
    return mkv2(-a.v[0], -a.v[1]);
}

static inline vec3 negv3(vec3 a) {
    return mkv3(-a.v[0], -a.v[1], -a.v[2]);
}

static inline vec4 negv4(vec4 a) {
    return mkv4(-a.v[0], -a.v[1], -a.v[2], -a.v[3]);
}

static inline float dotv2(vec2 a, vec2 b) {
    return a.v[0] * b.v[0] + a.v[1] * b.v[1];
}

static inline float dotv3(vec3 a, vec3 b) {
    return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

static inline float dotv4(vec4 a, vec4 b) {
    return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] + a.v[3] * b.v[3];
}

static inline vec2 addv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] + b.v[0];
    r.v[1] = a.v[1] + b.v[1];
    return r;
}

static inline vec3 addv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] + b.v[0];
    r.v[1] = a.v[1] + b.v[1];
    r.v[2] = a.v[2] + b.v[2];
    return r;
}

static inline vec4 addv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] + b.v[0];
    r.v[1] = a.v[1] + b.v[1];
    r.v[2] = a.v[2] + b.v[2];
    r.v[3] = a.v[3] + b.v[3];
    return r;
}

static inline vec2 subv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] - b.v[0];
    r.v[1] = a.v[1] - b.v[1];
    return r;
}

static inline vec3 subv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] - b.v[0];
    r.v[1] = a.v[1] - b.v[1];
    r.v[2] = a.v[2] - b.v[2];
    return r;
}

static inline vec4 subv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] - b.v[0];
    r.v[1] = a.v[1] - b.v[1];
    r.v[2] = a.v[2] - b.v[2];
    r.v[3] = a.v[3] - b.v[3];
    return r;
}

static inline vec2 mulv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] * b.v[0];
    r.v[1] = a.v[1] * b.v[1];
    return r;
}

static inline vec3 mulv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] * b.v[0];
    r.v[1] = a.v[1] * b.v[1];
    r.v[2] = a.v[2] * b.v[2];
    return r;
}

static inline vec4 mulv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] * b.v[0];
    r.v[1] = a.v[1] * b.v[1];
    r.v[2] = a.v[2] * b.v[2];
    r.v[3] = a.v[3] * b.v[3];
    return r;
}

static inline mat3 addm3(mat3 a, mat3 b) {
    return mkm3(
        addv3(a.c[0], b.c[0]),
        addv3(a.c[1], b.c[1]),
        addv3(a.c[2], b.c[2]));
}

static inline mat4 addm4(mat4 a, mat4 b) {
    return mkm4(
        addv4(a.c[0], b.c[0]),
        addv4(a.c[1], b.c[1]),
        addv4(a.c[2], b.c[2]),
        addv4(a.c[3], b.c[3]));
}

static inline vec3 mulm3v3(mat3 m, vec3 v) {
    return mkv3(
        m.c[0].v[0] * v.v[0] + m.c[1].v[0] * v.v[1] + m.c[2].v[0] * v.v[2],
        m.c[0].v[1] * v.v[0] + m.c[1].v[1] * v.v[1] + m.c[2].v[1] * v.v[2],
        m.c[0].v[2] * v.v[0] + m.c[1].v[2] * v.v[1] + m.c[2].v[2] * v.v[2]);
}

static inline vec4 mulm4v4(mat4 m, vec4 v) {
    return mkv4(
        m.c[0].v[0] * v.v[0] + m.c[1].v[0] * v.v[1] + m.c[2].v[0] * v.v[2] + m.c[3].v[0] * v.v[3],
        m.c[0].v[1] * v.v[0] + m.c[1].v[1] * v.v[1] + m.c[2].v[1] * v.v[2] + m.c[3].v[1] * v.v[3],
        m.c[0].v[2] * v.v[0] + m.c[1].v[2] * v.v[1] + m.c[2].v[2] * v.v[2] + m.c[3].v[2] * v.v[3],
        m.c[0].v[3] * v.v[0] + m.c[1].v[3] * v.v[1] + m.c[2].v[3] * v.v[2] + m.c[3].v[3] * v.v[3]);
}

static inline mat3 mulm3(mat3 a, mat3 b) {
    return mkm3(
        mulm3v3(a, b.c[0]),
        mulm3v3(a, b.c[1]),
        mulm3v3(a, b.c[2]));
}

static inline mat4 mulm4(mat4 a, mat4 b) {
    return mkm4(
        mulm4v4(a, b.c[0]),
        mulm4v4(a, b.c[1]),
        mulm4v4(a, b.c[2]),
        mulm4v4(a, b.c[3]));
}

static inline vec2 divv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] / b.v[0];
    r.v[1] = a.v[1] / b.v[1];
    return r;
}

static inline vec3 divv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] / b.v[0];
    r.v[1] = a.v[1] / b.v[1];
    r.v[2] = a.v[2] / b.v[2];
    return r;
}

static inline vec4 divv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] / b.v[0];
    r.v[1] = a.v[1] / b.v[1];
    r.v[2] = a.v[2] / b.v[2];
    r.v[3] = a.v[3] / b.v[3];
    return r;
}

static inline vec3 mulfv3(float f, vec3 v) {
    vec3 r = v;
    r.v[0] *= f;
    r.v[1] *= f;
    r.v[2] *= f;
    return r;
}

static inline vec4 mulfv4(float f, vec4 v) {
    vec4 r = v;
    r.v[0] *= f;
    r.v[1] *= f;
    r.v[2] *= f;
    r.v[3] *= f;
    return r;
}

static inline float distancev3(vec3 a, vec3 b) {
    vec3 d = subv3(a, b);
    return sqrtf(dotv3(d, d));
}

static inline float distancev4(vec4 a, vec4 b) {
    vec4 d = subv4(a, b);
    return sqrtf(dotv4(d, d));
}

static inline float lenv2(vec2 a) {
    return sqrtf(dotv2(a, a));
}

static inline float lenv3(vec3 a) {
    return sqrtf(dotv3(a, a));
}

static inline float lenv4(vec4 a) {
    return sqrtf(dotv4(a, a));
}

static inline vec2 normalizev2(vec2 a) {
    vec2 r = a;
    float d = sqrtf(dotv2(r, r));
    r.v[0] /= d;
    r.v[1] /= d;
    return r;
}

static inline vec3 normalizev3(vec3 a) {
    vec3 r = a;
    float d = sqrtf(dotv3(r, r));
    r.v[0] /= d;
    r.v[1] /= d;
    r.v[2] /= d;
    return r;
}

static inline vec4 normalizev4(vec4 a) {
    vec4 r = a;
    float d = sqrtf(dotv4(r, r));
    r.v[0] /= d;
    r.v[1] /= d;
    r.v[2] /= d;
    r.v[3] /= d;
    return r;
}

static inline vec3 crossv3(vec3 a, vec3 b) {
    return mkv3(
        a.v[1] * b.v[2] - a.v[2] * b.v[1],
        a.v[2] * b.v[0] - a.v[0] * b.v[2],
        a.v[0] * b.v[1] - a.v[1] * b.v[0]);
}

static inline mat3 outerv3(vec3 a, vec3 b) {
    return mkm3(
        mkv3(a.v[0] * b.v[0], a.v[0] * b.v[1], a.v[0] * b.v[2]),
        mkv3(a.v[1] * b.v[0], a.v[1] * b.v[1], a.v[1] * b.v[2]),
        mkv3(a.v[2] * b.v[0], a.v[2] * b.v[1], a.v[2] * b.v[2]));
}

static inline mat4 outerv4(vec4 a, vec4 b) {
    return mkm4(
        mkv4(a.v[0] * b.v[0], a.v[0] * b.v[1], a.v[0] * b.v[2], a.v[0] * b.v[3]),
        mkv4(a.v[1] * b.v[0], a.v[1] * b.v[1], a.v[1] * b.v[2], a.v[1] * b.v[3]),
        mkv4(a.v[2] * b.v[0], a.v[2] * b.v[1], a.v[2] * b.v[2], a.v[2] * b.v[3]),
        mkv4(a.v[3] * b.v[0], a.v[3] * b.v[1], a.v[3] * b.v[2], a.v[3] * b.v[3]));
}

static inline float minf(float a, float b) {
    return a < b ? a : b;
}

static inline float maxf(float a, float b) {
    return a > b ? a : b;
}

static inline float clampf(float a, float min, float max) {
    return minf(maxf(a, min), max);
}

static inline float smoothstepf(float min, float max, float a) {
    if (a < min) {
        return 0.0f;
    }
    if (a > max) {
        return 1.0f;
    }
    float t = (a - min) / (max - min);
    t = t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    t = clampf(t, 0.0f, 1.0f);
    return t;
}

static inline mat3 mkrotationm3(vec3 axis, float angle) {
    float c = cosf(angle);
    float nc = 1.0f - c;
    float s = sinf(angle);
    float v0 = axis.v[0];
    float v1 = axis.v[1];
    float v2 = axis.v[2];
    /* Formula copied from wikipedia
     * matrices are column-major so this looks transposed
     * with regards to what you'd find there */
    return mkm3(
        mkv3(v0 * v0 * nc + c, v1 * v0 * nc + v2 * s, v2 * v0 * nc - v1 * s),
        mkv3(v0 * v1 * nc - v2 * s, v1 * v1 * nc + c, v2 * v1 * nc + v0 * s),
        mkv3(v0 * v2 * nc + v1 * s, v1 * v2 * nc - v0 * s, v2 * v2 * nc + c));
}

static inline mat4 mkrotationm4(vec3 axis, float angle) {
    float c = cosf(angle);
    float nc = 1.0f - c;
    float s = sinf(angle);
    float v0 = axis.v[0];
    float v1 = axis.v[1];
    float v2 = axis.v[2];
    /* Formula copied from wikipedia
     * matrices are column-major so this looks transposed
     * with regards to what you'd find there */
    return mkm4(
        mkv4(v0 * v0 * nc + c, v1 * v0 * nc + v2 * s, v2 * v0 * nc - v1 * s, 0),
        mkv4(v0 * v1 * nc - v2 * s, v1 * v1 * nc + c, v2 * v1 * nc + v0 * s, 0),
        mkv4(v0 * v2 * nc + v1 * s, v1 * v2 * nc - v0 * s, v2 * v2 * nc + c, 0),
        mkv4(0, 0, 0, 1));
}

/* rotation matrix that will make d point the same direction as z */
/* both must be normalized */
static inline mat3 mkrotationalignm3(vec3 d, vec3 z) {
    vec3 v = crossv3(d, z);
    float c = dotv3(d, z);
    float k = 1.0f / (1.0f + c);
    return addm3(
        outerv3(mulfv3(k, v), v),
        mkm3(mkv3(      c,  v.v[2], -v.v[1]),
             mkv3(-v.v[2],       c,  v.v[0]),
             mkv3( v.v[1], -v.v[0],       c)));
}

static inline mat4 mktranslationm4(vec3 v) {
    return mkm4(
        mkv4(1, 0, 0, 0),
        mkv4(0, 1, 0, 0),
        mkv4(0, 0, 1, 0),
        mkv4(v.v[0], v.v[1], v.v[2], 1));
}

static inline double clamp(double a, double min, double max) {
    return
        a < min ? min :
        a > max ? max :
                  a;
}

static inline double smoothstep(double min, double max, double a) {
    if (a < min) {
        return 0.0f;
    }
    if (a > max) {
        return 1.0f;
    }
    double t = (a - min) / (max - min);
    t = t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    t = clamp(t, 0.0f, 1.0f);
    return t;
}

