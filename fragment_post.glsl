
/* gradient */
vec3 grad(vec3 p) {
    float eps = 0.0001;
    return normalize(vec3(
                dist_object(p - vec3(eps, 0.0, 0.0)) - dist_object(p + vec3(eps, 0.0, 0.0)),
                dist_object(p - vec3(0.0, eps, 0.0)) - dist_object(p + vec3(0.0, eps, 0.0)),
                dist_object(p - vec3(0.0, 0.0, eps)) - dist_object(p + vec3(0.0, 0.0, eps))));
}

/* trace from point p along ray r */
vec4 trace(vec3 p, vec3 r) {
    vec3 p1 = p;
    float d = dist_object(p);
    float epsilon = 1.0e-05;
    // Kalman summation
    float dsum = 0.0;
    float dsumerr = 0.0;
    float tmp;
    for (int i = 0; i < 512 && d >= epsilon; i++) {
        // escape if too long
        if (dsum > 16) {
            return vec4(p1, 0.0);
        }
        tmp = dsum;
        dsum = dsum + d;
        tmp = tmp - (dsum - d);
        dsumerr = dsumerr + tmp;
        p1 = dsumerr * r + dsum * r + p;
        d = dist_object(p1);
    }
    if (d > epsilon) {
        return vec4(p1, 0.0);
    } else {
        // woop woop
        for (int j = 0; j < 16 && d < 0; j++) {
            tmp = dsum;
            dsum = dsum + d;
            tmp = tmp - (dsum - d);
            dsumerr = dsumerr + tmp;
            p1 = dsumerr * r + dsum * r + p;
            d = dist_object(p1);
        }
        p1 = dsumerr * r + dsum * r + p;
        return vec4(p1, 1.0);
    }
}

vec3 srgb(float r, float g, float b) {
    vec3 c = vec3(r / 255.0, g / 255.0, b / 255.0);
    return pow(c, vec3(2.2, 2.2, 2.2));
}

vec3 texmex(vec3 p, vec3 n) {
//    return (vec3(1.0, 1.0, 1.0) + n) * 0.5;
//    return vec3(1.0, 1.0, 1.0);
    
    // glTexGen GL_SPHERE_MAP
    vec3 u = normalize(p);
    vec3 f = u - 2 * dot(n, u) * n;
    float m = 2 * sqrt(f.x * f.x + f.y * f.y + (f.z + 1) * (f.z + 1));
    vec2 t = f.xy / m + vec2(0.5, 0.5);

    // :/
//    float sblue = sin(dot(t, t) * 18 * TAU);
//    sblue = (sblue + 1) * 0.5;
    //float sblue = step(0.5, fract((t.x * t.x + t.y * t.y) * 1.5));
    //float sblue = step(0.5, fract((t.x + t.y) * 1.5));
    //return mix(srgb(89, 132, 50), srgb(148, 174, 22), sblue);
    //return mix(srgb(246, 200, 44), srgb(250, 236, 147), sblue);
    //return vec3(t, 0.3 + 0.4 * timing2(12345));
    float c = distance(vec2(0.5, 0.5), t) * 1.8;
    return vec3(1.0, 1.0 - c, 1.0 - c);
}

const vec3 light1 = normalize(vec3(-0.0, -0.2, -0.0));
const vec3 light2 = normalize(vec3(0.1, -0.1, -0.0));
vec3 light3_pos = vec3(-0.2, 0, 2.5);
vec3 shade(vec3 p) {
    vec3 n = grad(p);
    vec3 light3 = normalize(p - light3_pos);
    vec4 m1 = trace(p - light1 * 0.05, -light1);
    vec4 m2 = trace(p - light2 * 0.05, -light2);
    vec4 m3 = trace(light3_pos, light3);
    float factor1 = (1.0 - m1.w) * dot(n, light1);
    float factor2 = (1.0 - m2.w) * dot(n, light2);
    m3.xyz -= p;
    float factor3 = (1.0 - step(0.01, dot(m3.xyz, m3.xyz)) * m3.w) * dot(n, light3);

    vec3 c = texmex(p, n);

/*
    vec3 light1c = vec3(1, 1, 1);
    vec3 light2c = vec3(1, 1, 1);
    vec3 light3c = vec3(1, 1, 1);
*/
    vec3 light1c = vec3(0.9, 0.1, 0.1);
    vec3 light2c = vec3(0.1, 0.9, 0.1);
    vec3 light3c = vec3(0.1, 0.1, 0.9);
    // no light shadows only
//   return c * ((2.0 - m1.w - m2.w) * 2.0 / 3.0 + 1.0 / 3.0);
    // three lights
     return min((
     max(factor1, 0.0) * light1c +
     max(factor2, 0.0) * light2c +
     max(factor3, 0.0) * light3c
     ) / (light1c + light2c + light3c) * c, c);
    // debug shadows
//    return vec3(m1.w, m2.w, 0.0);
}

vec4 go(vec3 p, vec3 ray) {
    // wavy effect1
    
    /*
    float phi = TAU * 2 * (ray.x + ray.y);
    p.z += sin(phi) * cos(phi) * 0.01;
    */

    // wavy effect2
    /*
    float phi = TAU * timing2(15000);
    ray.x *= sin(phi + ray.x);
    */

    vec4 q = trace(p, ray);
    vec3 result;
    if (q.w < 1.0) {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
    p = q.xyz;
    return vec4(shade(p), 1.0);
}

void main() {
    //rand_state = uint(millis) + uint((pixelcenter.x + pixelcenter.y) * 1000);
    vec3 p = vec3(0.0, 0.0, 1.3);
    vec3 t = vec3(pixelcenter, 0.98);

    light3_pos = (camera * vec4(light3_pos, 1.0)).xyz;
    p = (camera * vec4(p, 1.0)).xyz;

    vec3 tr;
    vec3 ray = normalize((camera * vec4(t, 1.0)).xyz - p);
    vec4 result = go(p, ray);
/*
    // the number of iterations plus one must be
    // divided by below.
    // anti-aliasing is turned off right now because it murders performance
    for (i = 0; i < 0; i++) {
        tr = t + vec3(pixel * rand2(), 0.0);
        tr = (camera * vec4(tr, 1.0)).xyz;
        ray = normalize(tr - p);
        result += go(p, ray);
    }
*/
    vec4 c = result;
    vec2 texcoord = (vec2(1, 1) + screenpixel) * 0.5;
    float factor = 1.0 / MOTIONBLUR_COEFFICIENT;
    for (int i = 0, j = currentFramebuffer;
            i < MOTIONBLUR_FACTOR - 1; i++) {
        j = (j + MOTIONBLUR_FACTOR - 1) % MOTIONBLUR_FACTOR;
        c += texture(framessampler[j], texcoord) * factor;
        factor /= MOTIONBLUR_COEFFICIENT;
    }
    c *= (MOTIONBLUR_COEFFICIENT - 1) / (MOTIONBLUR_COEFFICIENT - pow(MOTIONBLUR_COEFFICIENT, -float(MOTIONBLUR_FACTOR)));
    colorObject = result;
    // gamma correction for standard monitor
    float g = 1.0 / 2.2;
    colorBackLeft = pow(c, vec4(g, g, g, 1.0));
}
