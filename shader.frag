#version 150

uniform int millis;
in vec3 inray;
out vec4 color;

vec3 ray;

float TAU = 6.28318530717958647692;

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

float cylinderx(vec3 p, vec3 c, float h, float r) {
    vec3 q = p - c;
    return max(max(-h - q.x, q.x - h), sqrt(dot(q.yz, q.yz)) - r);
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

float dist_object(vec3 p) {
    float t = timing2(4000);
    vec3 centre = vec3(0.2, 0.0, 0.0);

    // tiling effect
    /*
    vec3 vmin = vec3(-1.0, -1.0, -1.0);
    vec3 vmax = vec3(1.0, 1.0, 1.0);
    p = vmin + fract((p - vmin) / (vmax - vmin)) * (vmax - vmin);
    */
    //OMGWTF!?
    /*
    p = p + vec3(sin(TAU * 0.5 * fract(p.x + t)),
                 sin(TAU * 0.5 * fract(p.y + t)),
                 sin(TAU * 0.5 * fract(p.z + t)));
    */
//    return mixfix(sphere(p, centre, 0.25), cube(p), t);
//    return mixfix(cube(p, centre, 0.2), sphere2(p, centre, 0.5), t);
// 3-way morph and moving over a plane with shadow
/*
    centre = centre + timing2(11123) * vec3(0.0, -0.75, 0.0);
    return min(mixfix(mixfix(cube(p, centre, 0.25), cylinderx(p, centre, 0.25, 0.25), t * 2.0),
        sphere(p, centre, 0.4), (t - 0.5) * 2.0),
        cube(p, vec3(0.0, -1.0, 0.0), vec3(100.0, 0.5, 100.0)));
*/
    vec3 disp = timing2(12345) * vec3(0.5, 0.0, 0.0);
    //return min(min(sphere(p, centre + disp, 0.5), sphere(p, centre - disp, 0.5)), cube(p, vec3(0.0, -1.0, 0.0), vec3(100.0, 0.5, 100.0)));
    return min(cube(p, centre, 0.2), cube(p, vec3(0.0, -1.0, 0.0), vec3(100.0, 0.5, 100.0)));
    // return min(min(cylinderx(p, centre, 0.3, 0.2),
    //             sphere(p, centre + vec3(0.0, 0.4, 0.0), 0.4)),
    //             cube(p, centre + vec3(0.0, -0.5, -0.2), 0.4));
//    return sphere(p, centre, 0.3);
//    return mixfix(cube(p, vec3(0.0, 1.0, 0.0), 0.8), cube(p, vec3(0.4, 0.4, 0.4), 0.1), t);

//     return cube(p + vec3(sin(3.141259 * fract(p.x + t)),
//                          sin(3.141259 * fract(p.y + t)),
//                          sin(3.141259 * fract(p.z + t))));

}


/* trace from point p along ray r */
vec4 trace(vec3 p, vec3 r) {
    vec3 p1 = p;
    float d = dist_object(p);
    float epsilon = 4.0e-07;
    for (int i = 0; i < 1024; i++) {
        // escape if too long
        if (dot(p - p1, r) > 16) {
            break;
        }
        if (d > epsilon) {
            p += d * r;
            d = dist_object(p);
        } else {
            break;
        }
    }
    if (d > epsilon) {
        return vec4(d, d, d, 0);
    }
    p += d * r;
    return vec4(p, 1);
}

vec3 shade(vec3 p, vec3 c) {
    vec3 t1 = cross(ray, vec3(1, 0, 0));
    vec3 light1 = normalize(vec3(-0.5, -0.2, -0.1));
    vec3 light2 = normalize(vec3(0.1, -0.1, -1.0));
    if (dot(t1, t1) < 0.001) {
        t1 = cross(ray, vec3(0, 1, 0));
    }
    t1 = normalize(t1);
    vec3 t2 = normalize(cross(ray, t1));
    vec3 p1 = p + 0.0001 * t1;
    vec3 p2 = p + 0.0001 * t2;
    vec4 q1 = trace(p1 - ray, ray);
    vec4 q2 = trace(p2 - ray, ray);
    if (q1.w > 0) {
        p1 = q1.xyz;
    }
    if (q2.w > 0) {
        p2 = q2.xyz;
    }
    vec3 n = normalize(cross(p1 - p, p2 - p));
    vec4 m1 = trace(p - light1 * 0.00001, -light1);
    vec4 m2 = trace(p - light2 * 0.00001, -light2);
    float factor1 = dot(n, light1);
    float factor2 = dot(n, light2);
    if (m1.w > 0.0) {
        factor1 = 0.0;
    }
    if (m2.w > 0.0) {
        factor2 = 0.0;
    }
    return min((max(factor1, 0.0) +
           max(factor2, 0.0)) * 0.5 * c, c);
//    return (vec3(1.0, 1.0, 1.0) + normalize(cross(p1 - p, p2 - p))) * 0.5;
}

void main() {
    vec3 p = vec3(0.0, 0.0, 2.0);
    ray = inray;

    // wavy effect1
    /*
    float phi = TAU * 3 * timing2(15000) * (timing2(10000) + ray.x + ray.y);
    p.z += sin(phi) * cos(phi);
    */

    // wavy effect2
    /*
    float phi = TAU * timing2(15000);
    ray.x *= sin(phi + ray.x);
    */

    // camera rotation - timing sets speed for one rotation in ms
    float angle = TAU * timing(47000);
    vec3 axis = normalize(vec3(-0.2, 1.0, 0.3));
    mat3x3 rotmat = mat3(vec3(1.0, 0.0, 0.0),
                         vec3(0.0, 1.0, 0.0),
                         vec3(0.0, 0.0, 1.0));
    rotmat = rotmat * cos(angle);
    rotmat = rotmat + sin(angle) * mat3(vec3(0.0, axis.z, -axis.y),
                                        vec3(-axis.z, 0.0, axis.x),
                                        vec3(axis.y, -axis.x, 0.0));
    rotmat = rotmat + (1.0 - cos(angle)) * outerProduct(axis, axis);
    p = rotmat * p;
    ray = rotmat * ray;

    vec4 q = trace(p, ray);
    if (q.w < 1.0) {
        discard;
    }
    p = q.xyz;
    color = vec4(shade(p, vec3(1.0, 1.0, 1.0)), 1.0);
    color = pow(color, vec4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0));
}

