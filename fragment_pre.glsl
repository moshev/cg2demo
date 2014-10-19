#version 150

const float TAU = 6.2831853;

uniform int millis;
uniform int currentFramebuffer;
uniform mat4 camera;

#define MOTIONBLUR_COEFFICIENT 8.0
// newlines left as space to insert motionblur factor
// keep them at least 3!
#define MOTIONBLUR_FACTOR 






uniform sampler2D framessampler[MOTIONBLUR_FACTOR];

in float millis2;
// ray
centroid in vec2 pixelcenter;
centroid in vec2 screenpixel;

/*
// pixel size
flat in vec2 pixel;
*/

out vec4 colorBackLeft;
out vec4 colorObject;

/*
// set this to something depending on
// both time and pixel
uint rand_state = 0u;

float rand() {
    rand_state = ((rand_state * 1664525u + 1013904223u) >> 8) & 0xFFFFFFu;
    return float(rand_state) / float(0xFFFFFF);
}

vec2 rand2_state = vec2(-0.75, 0.75);
vec2 rand2_state_m = vec2(-1.0, 1.0);

void srand2(int seed) {
}

vec2 rand2() {
    rand2_state *= rand2_state_m;
    rand2_state_m *= vec2(-1.0, -1.0);
    return rand2_state;
}
*/
float mixfix(float a, float b, float t) {
    // this piece is nonsensical but without it
    // we get a black screen, fuck you nVidia
    // fuck you with a rusty rake
    // (pls fix your floating point)
    float u;
    t = clamp(t, 0.0, 1.0);
    u = 1.0 - t;
    u = clamp(u, 0.0, 1.0);
    return a * u + b * t;
}

/*rotation matrix that will make d point the same direction as z*/
/*both must be normalized*/
/*
mat3x3 rotationAlign(vec3 d, vec3 z) {
    vec3 v = cross(z, d);
    float c = dot(z, d);
    float k = 1.0f / (1.0f + c);
    return k * outerProduct(v, v) +
        mat3x3(c, v.z, -v.y,
               -v.z, c, v.x,
               v.y, -v.x, c);
}
*/

mat4 mkrotationm4(vec3 axis, float angle) {
    float c = cos(angle);
    float nc = 1.0 - c;
    float s = sin(angle);
    float v0 = axis.x;
    float v1 = axis.y;
    float v2 = axis.z;
    /* Formula copied from wikipedia
     * matrices are column-major so this looks transposed
     * with regards to what you'd find there */
    return mat4(
        v0 * v0 * nc + c, v1 * v0 * nc + v2 * s, v2 * v0 * nc - v1 * s, 0,
        v0 * v1 * nc - v2 * s, v1 * v1 * nc + c, v2 * v1 * nc + v0 * s, 0,
        v0 * v2 * nc + v1 * s, v1 * v2 * nc - v0 * s, v2 * v2 * nc + c, 0,
        0, 0, 0, 1);
}

float plane(vec3 p, vec3 c, vec3 n) {
    return dot(n, p) - dot(n, c);
}

/*cube with 3 lengths*/
float cube(vec3 p, vec3 c, vec3 vr) {
    vec3 bmin = c - vr;
    vec3 bmax = c + vr;
    vec3 dmin = bmin - p;
    vec3 dmax = p - bmax;
    vec3 max1 = max(dmin, dmax);
    vec2 max2 = max(max1.xy, max1.z);
    return max(max2.x, max2.y);
}

/*proper cube*/
float cube(vec3 p, vec3 c, float r) {
    vec3 vr = vec3(r, r, r);
    return cube(p, c, vr);
}

/*sphere*/
float sphere(vec3 p, vec3 c, float r) {
    return distance(c, p) - r;
}

/*torus*/
/*rc - radius to centre of tube*/
/*rt - radius of tube*/
float torus(vec3 p, vec3 c, vec3 n, float rc, float rt) {
    // equation is
    // (rmax - sqrt(dot(p.xy))) ** 2 + z**2 - rmin**2
    // for torus symmetric around z
    float z = dot(p, n) - dot(c, n);
    vec3 p1 = p - z * n;
    float xy2 = dot(p1 - c, p1 - c);
    float b = rc - sqrt(xy2);
    return sqrt(b * b + z * z) - rt;
}

/*
float cylinderx(vec3 p, vec3 c, float h, float r) {
    vec3 q = p - c;
    return max(max(-h - q.x, q.x - h), sqrt(dot(q.yz, q.yz)) - r);
}

float cylindery(vec3 p, vec3 c, float h, float r) {
    vec3 q = p - c;
    return max(max(-h - q.y, q.y - h), sqrt(dot(q.xz, q.xz)) - r);
}
*/

/*cylinder with spherical caps at ends*/
/* a, b - centres of the caps, r - radius */
float cylinder_caps(vec3 p, vec3 a, vec3 b, float r) {
    vec3 n = normalize(b - a);
    vec3 p1 = p - a;
    float d = dot(n, p1);
    vec3 c = d * n;
    if (dot(n, c) < 0.0f) {
        return sphere(p, a, r);
    }
    if (dot(n, c) > distance(a, b)) {
        return sphere(p, b, r);
    }
    float daxis = length(p1 - d * n);
    return daxis - r;
}

/* tile vec3 around the centre with radius r */
vec3 tile(vec3 p, vec3 r) {
    return 2 * (fract((p + r) / (2 * r)) - vec3(0.5, 0.5, 0.5)) * r;
}

// 0.0 - 1.0
float timing(int p) {
    return float(int(millis) % p) / float(p - 1);
}

// 0.0 - 1.0 - 0.0
float timing2(int p) {
    float t = timing(p);
    return 2.0 * (0.5 - abs(t - 0.5));
}

// 0.0 - 1.0
float m2timing(int p) {
    return float(int(millis2) % p) / float(p - 1);
}

// 0.0 - 1.0 - 0.0
float m2timing2(int p) {
    float t = m2timing(p);
    return 2.0 * (0.5 - abs(t - 0.5));
}

mat4 mkcamera() {
    float rotf = m2timing(32000);
    float trf = m2timing(17000);
    rotf = smoothstep(0, 1, rotf);
    trf = smoothstep(0, 1, trf);

    //return mkrotationm4(mkv3(0, 1, 0), trf * TAU);
    ///*
    return camera
        * mkrotationm4(vec3(0, 1, 0), rotf * TAU)
        * mkrotationm4(
                normalize(vec3(
                        0.5f + 0.5f * cos(trf * TAU),
                        1.0f,
                        0.5f + 0.5f * sin(trf * TAU))),
                trf * TAU);
}
