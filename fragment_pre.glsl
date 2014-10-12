#version 150

const float TAU = 6.28318530717958647692;

uniform int millis;
uniform mat4 camera;

// ray
centroid in vec2 pixelcenter;

// pixel size
flat in vec2 pixel;

out vec4 color;

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
mat3x3 rotationAlign(vec3 d, vec3 z) {
    vec3 v = cross(z, d);
    float c = dot(z, d);
    float k = 1.0f / (1.0f + c);
    return k * outerProduct(v, v) +
        mat3x3(c, v.z, -v.y,
               -v.z, c, v.x,
               v.y, -v.x, c);
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

/*2 spheres*/
float sphere2(vec3 p, vec3 c, float r) {
    vec3 c1 = c;
    vec3 c2 = c;
    c1.x -= r * 0.5;
    c2.x += r * 0.5;
    float r1 = r * 0.5;
    return min(sphere(p, c1, r1), sphere(p, c2, r1));
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

float cylinderx(vec3 p, vec3 c, float h, float r) {
    vec3 q = p - c;
    return max(max(-h - q.x, q.x - h), sqrt(dot(q.yz, q.yz)) - r);
}

float cylindery(vec3 p, vec3 c, float h, float r) {
    vec3 q = p - c;
    return max(max(-h - q.y, q.y - h), sqrt(dot(q.xz, q.xz)) - r);
}

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

// 0.0 - 1.0
float timing(int period) {
    return float(millis % period) / float(period - 1);
}

// 0.0 - 1.0 - 0.0
float timing2(int period) {
    float t = timing(period);
    return 2.0 * (0.5 - abs(t - 0.5));
}
