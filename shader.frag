#version 150

uniform int millis;
in vec3 inray;
out vec4 color;

vec3 ray;

/*cube*/
float cube(vec3 p) {
    vec3 bmin = vec3(-0.7, -0.7, -0.7);
    vec3 bmax = vec3(-0.2, -0.2, -0.2);
    vec3 dmin = bmin - p;
    vec3 dmax = p - bmax;
    vec3 max1 = max(dmin, dmax);
    vec2 max2 = max(max1.xy, max1.z);
    return max(max2.x, max2.y);
}


/*sphere*/
float sphere(vec3 p, vec3 c, float r) {
    return distance(c, p) - r;
}

float timing(int period) {
    return float(millis % period) / float(period - 1);
}

float dist_object(vec3 p) {
    float t = timing(4000);
    t = 2.0 * abs(t - 0.5);
    float u = (1.0 - t);
    // this piece is nonsensical but without it
    // we get a black screen, fuck you nVidia
    // fuck you with a rusty rake
    if (u < 0.0) {
        u = 0.0;
    }
    vec3 centre = vec3(-0.2, -0.2, -0.4);
    return (u * sphere(p, centre, 0.25) + t * cube(p));
/*
    return cube(p + vec3(sin(3.141259 * fract(p.x + t)),
                         sin(3.141259 * fract(p.y + t)),
                         sin(3.141259 * fract(p.z + t))));
                        */
}


vec4 trace(vec3 p) {
    float d = dist_object(p);
    float epsilon = 4.0e-07;
    for (int i = 0; i < 1024; i++) {
        if (d > epsilon) {
            p += d * ray;
            d = dist_object(p);
        }
    }
    if (d > epsilon) {
        return vec4(d, d, d, 0);
    }
    p += d * ray;
    return vec4(p, 1);
}

vec3 shade(vec3 p, vec3 c) {
    vec3 t1 = cross(ray, vec3(1, 0, 0));
    vec3 light1 = normalize(vec3(-0.5, -0.2, -0.1));
    vec3 light2 = normalize(vec3(0.1, -0.0, -1.0));
    if (dot(t1, t1) < 0.001) {
        t1 = cross(ray, vec3(0, 1, 0));
    }
    t1 = normalize(t1);
    vec3 t2 = normalize(cross(ray, t1));
    vec3 p1 = p + 0.0001 * t1;
    vec3 p2 = p + 0.0001 * t2;
    vec4 q1 = trace(p1 - ray);
    vec4 q2 = trace(p2 - ray);
    if (q1.w > 0) {
        p1 = q1.xyz;
    }
    if (q2.w > 0) {
        p2 = q2.xyz;
    }
    vec3 n = normalize(cross(p1 - p, p2 - p));
    return min((max(dot(n, light1), 0.0) +
           max(dot(n, light2), 0.0)) * 0.5 * c, c);
//    return (vec3(1.0, 1.0, 1.0) + normalize(cross(p1 - p, p2 - p))) * 0.5;
}

void main() {
    vec3 p = vec3(0.0, 0.0, 1.0);
    float angle = 6.28318530717958647692 * timing(20000);
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
    ray = rotmat * inray;
    vec4 q = trace(p);
    if (q.w < 1.0) {
        discard;
    }
    p = q.xyz;
    color = vec4(shade(p, vec3(1.0, 1.0, 1.0)), 1.0);
    color = pow(color, vec4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0));
}

