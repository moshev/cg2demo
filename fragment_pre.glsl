#version 150

const float TAU = 6.2831853;

uniform int millis;
uniform int currentFramebuffer;
uniform mat4 camera;

#define MOTIONBLUR_COEFFICIENT 2.5
// newlines left as space to insert motionblur factor
// keep them at least 3!
#define MOTIONBLUR_FACTOR 






uniform sampler2D framessampler[MOTIONBLUR_FACTOR];

// ray
centroid in vec2 pixelcenter;
centroid in vec2 screenpixel;

out vec4 colorBackLeft;
out vec4 colorObject;

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
