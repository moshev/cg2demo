#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define __USE_MISC 1
#include <math.h>

#define VEC(n) typedef struct t_vec##n { float v[n]; } vec##n;

/* NOTE: Matrices are column-major like in OpenGL (and fortran) */
#define MAT(n) typedef struct t_mat##n { float v[n * n]; } mat##n;

VEC(2);
VEC(3);
VEC(4);
MAT(3);
MAT(4);

vec2 mkv2(float v0, float v1) {
    vec2 r = { { v0, v1 } };
    return r;
}

vec3 mkv3(float v0, float v1, float v2) {
    vec3 r = { { v0, v1, v2 } };
    return r;
}

vec4 mkv4(float v0, float v1, float v2, float v3) {
    vec4 r = { { v0, v1, v2, v3 } };
    return r;
}

mat3 mkm3(vec3 c0, vec3 c1, vec3 c2) {
	/* column-major, so the actual matrix
	 * is the transpose of what you're looking at*/
    mat3 r = { { c0.v[0], c0.v[1], c0.v[2],
                 c1.v[0], c1.v[1], c1.v[2],
                 c2.v[0], c2.v[1], c2.v[2] } };
    return r;
}

mat4 mkm4(vec4 c0, vec4 c1, vec4 c2, vec4 c3) {
	/* column-major, so the actual matrix
	 * is the transpose of what you're looking at*/
    mat4 r = { { c0.v[0], c0.v[1], c0.v[2], c0.v[3],
                 c1.v[0], c1.v[1], c1.v[2], c1.v[3],
                 c2.v[0], c2.v[1], c2.v[2], c2.v[3],
                 c3.v[0], c3.v[1], c3.v[2], c3.v[3] } };
    return r;
}

const float TAU = 6.283185307179586476925286766559f;

vec2 negv2(vec2 a) {
    return mkv2(-a.v[0], -a.v[1]);
}

vec3 negv3(vec3 a) {
    return mkv3(-a.v[0], -a.v[1], -a.v[2]);
}

vec4 negv4(vec4 a) {
    return mkv4(-a.v[0], -a.v[1], -a.v[2], -a.v[3]);
}

float dotv2(vec2 a, vec2 b) {
    return a.v[0] * b.v[0] + a.v[1] * b.v[1];
}

float dotv3(vec3 a, vec3 b) {
    return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

float dotv4(vec4 a, vec4 b) {
    return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] + a.v[3] * b.v[3];
}

vec2 addv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] + b.v[0];
    r.v[1] = a.v[1] + b.v[1];
    return r;
}

vec3 addv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] + b.v[0];
    r.v[1] = a.v[1] + b.v[1];
    r.v[2] = a.v[2] + b.v[2];
    return r;
}

vec4 addv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] + b.v[0];
    r.v[1] = a.v[1] + b.v[1];
    r.v[2] = a.v[2] + b.v[2];
    r.v[3] = a.v[3] + b.v[3];
    return r;
}

vec2 subv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] - b.v[0];
    r.v[1] = a.v[1] - b.v[1];
    return r;
}

vec3 subv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] - b.v[0];
    r.v[1] = a.v[1] - b.v[1];
    r.v[2] = a.v[2] - b.v[2];
    return r;
}

vec4 subv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] - b.v[0];
    r.v[1] = a.v[1] - b.v[1];
    r.v[2] = a.v[2] - b.v[2];
    r.v[3] = a.v[3] - b.v[3];
    return r;
}

vec2 mulv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] * b.v[0];
    r.v[1] = a.v[1] * b.v[1];
    return r;
}

vec3 mulv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] * b.v[0];
    r.v[1] = a.v[1] * b.v[1];
    r.v[2] = a.v[2] * b.v[2];
    return r;
}

vec4 mulv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] * b.v[0];
    r.v[1] = a.v[1] * b.v[1];
    r.v[2] = a.v[2] * b.v[2];
    r.v[3] = a.v[3] * b.v[3];
    return r;
}

vec3 mulm3v3(mat3 m, vec3 v) {
    return mkv3(
            m.v[0] * v.v[0] + m.v[3] * v.v[1] + m.v[6] * v.v[2],
            m.v[1] * v.v[0] + m.v[4] * v.v[1] + m.v[7] * v.v[2],
            m.v[2] * v.v[0] + m.v[5] * v.v[1] + m.v[8] * v.v[2]);
}

vec4 mulm4v4(mat4 m, vec4 v) {
    return mkv4(
            m.v[ 0] * v.v[0] + m.v[ 4] * v.v[1] + m.v[ 8] * v.v[2] + m.v[12] * v.v[3],
            m.v[ 1] * v.v[0] + m.v[ 5] * v.v[1] + m.v[ 9] * v.v[2] + m.v[13] * v.v[3],
            m.v[ 2] * v.v[0] + m.v[ 6] * v.v[1] + m.v[10] * v.v[2] + m.v[14] * v.v[3],
            m.v[ 3] * v.v[0] + m.v[ 7] * v.v[1] + m.v[11] * v.v[2] + m.v[15] * v.v[3]);
}

vec2 divv2(vec2 a, vec2 b) {
    vec2 r;
    r.v[0] = a.v[0] / b.v[0];
    r.v[1] = a.v[1] / b.v[1];
    return r;
}

vec3 divv3(vec3 a, vec3 b) {
    vec3 r;
    r.v[0] = a.v[0] / b.v[0];
    r.v[1] = a.v[1] / b.v[1];
    r.v[2] = a.v[2] / b.v[2];
    return r;
}

vec4 divv4(vec4 a, vec4 b) {
    vec4 r;
    r.v[0] = a.v[0] / b.v[0];
    r.v[1] = a.v[1] / b.v[1];
    r.v[2] = a.v[2] / b.v[2];
    r.v[3] = a.v[3] / b.v[3];
    return r;
}

float distancev3(vec3 a, vec3 b) {
    vec3 d = subv3(a, b);
    return sqrtf(dotv3(d, d));
}

float distancev4(vec4 a, vec4 b) {
    vec4 d = subv4(a, b);
    return sqrtf(dotv4(d, d));
}

float lenv2(vec2 a) {
    return sqrtf(dotv2(a, a));
}

float lenv3(vec3 a) {
    return sqrtf(dotv3(a, a));
}

float lenv4(vec4 a) {
    return sqrtf(dotv4(a, a));
}

vec2 normalizev2(vec2 a) {
    vec2 r = a;
    float d = sqrtf(dotv2(r, r));
    r.v[0] /= d;
    r.v[1] /= d;
    return r;
}

vec3 normalizev3(vec3 a) {
    vec3 r = a;
    float d = sqrtf(dotv3(r, r));
    r.v[0] /= d;
    r.v[1] /= d;
    r.v[2] /= d;
    return r;
}

vec4 normalizev4(vec4 a) {
    vec4 r = a;
    float d = sqrtf(dotv4(r, r));
    r.v[0] /= d;
    r.v[1] /= d;
    r.v[2] /= d;
    r.v[3] /= d;
    return r;
}

float minf(float a, float b) {
    return a < b ? a : b;
}

float maxf(float a, float b) {
    return a > b ? a : b;
}

float clampf(float a, float min, float max) {
    return minf(maxf(a, min), max);
}

float smoothstep(float min, float max, float a) {
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

/*cube with 3 lengths*/
float cube(vec3 p, vec3 c, vec3 vr) {
    vec3 bmin = subv3(c, vr);
    vec3 bmax = addv3(c, vr);
    vec3 dmin = subv3(bmin, p);
    vec3 dmax = subv3(p, bmax);
    for (int i = 0; i < 3; i++) {
        if (dmax.v[i] < dmin.v[i])
            dmax.v[i] = dmin.v[i];
    }
    float r = dmax.v[0];
    if (r < dmax.v[1])
        r = dmax.v[1];
    if (r < dmax.v[2])
        r = dmax.v[2];
    return r;
}

/*sphere*/
float sphere(vec3 p, vec3 c, float r) {
    return distancev3(c, p) - r;
}

float cylinderx(vec3 p, vec3 c, float h, float r) {
    vec3 q = subv3(p, c);
    vec2 qyz;
    qyz.v[0] = q.v[1];
    qyz.v[1] = q.v[2];
    return maxf(maxf(-h - q.v[0], q.v[0] - h), sqrtf(dotv2(qyz, qyz)) - r);
}

float cylindery(vec3 p, vec3 c, float h, float r) {
    vec3 q = subv3(p, c);
    vec2 qxz;
    qxz.v[0] = q.v[0];
    qxz.v[1] = q.v[2];
    return maxf(maxf(-h - q.v[1], q.v[1] - h), sqrtf(dotv2(qxz, qxz)) - r);
}

/*cylinder with spherical caps at ends*/
/* a, b - centres of the caps, r - radius */
float cylinder_spherical_caps(vec3 p, vec3 a, vec3 b, float r) {
    vec3 n = normalizev3(subv3(b, a));
    vec3 p1 = subv3(p, a);
    float d = dotv3(n, p1);
    vec3 c = mulv3(n, mkv3(d, d, d));
    if (dotv3(n, c) < 0.0f) {
        return sphere(p, a, r);
    }
    if (dotv3(n, c) > distancev3(a, b)) {
        return sphere(p, b, r);
    }
    float daxis = lenv3(subv3(p1, mulv3(n, mkv3(d, d, d))));
    return daxis - r;
}

float dist_object1(vec3 p, uint64_t ticks) {
    (void) ticks;
    float t = smoothstep(-0.25f, 0.25f, p.v[1]);
    vec3 centre = { {0.2f, 0.0f, 0.0f} };
    float cx = cylinderx(p, centre, 0.5f, 0.5f);
    float cy = cylindery(p, centre, 0.4f, 0.25f);
    return t * cy + (1.0f - t) * cx;
}

float dist_object2(vec3 p, uint64_t ticks) {
    (void) ticks;
    return cylinder_spherical_caps(p, mkv3(-1.0f, 0.0f, 0.0f), mkv3(1.0f, 0.0f, 0.0f), 0.2f);
}

float dist_object3(vec3 p, uint64_t ticks) {
    float t = (ticks % 700) / 699.0f;
    t = (sinf(t * TAU) + 1.0f) * 0.5f;
    return
             t * cylinder_spherical_caps(p, mkv3(-1.0f, 0.0f, 0.0f), mkv3(1.0f, 0.0f, 0.0f), 0.6f) +
    (1.0f - t) * cylinder_spherical_caps(p, mkv3(0.0f, -1.0f, 0.0f), mkv3(0.0f, 1.0f, 0.0f), 0.1f);
}

#define dist_object dist_object3

vec3 grad(vec3 p, uint64_t ticks) {
    float eps = 1.0e-4f;
    vec3 dx = mkv3(eps, 0.f, 0.f);
    vec3 dy = mkv3(0.f, eps, 0.f);
    vec3 dz = mkv3(0.f, 0.f, eps);
    return normalizev3(mkv3(
                dist_object(subv3(p, dx), ticks) - dist_object(addv3(p, dx), ticks),
                dist_object(subv3(p, dy), ticks) - dist_object(addv3(p, dy), ticks),
                dist_object(subv3(p, dz), ticks) - dist_object(addv3(p, dz), ticks)));
}

#define WOOPWOOP 0
vec4 trace(vec3 p, vec3 r, uint64_t ticks) {
    vec3 p1;
    float d = dist_object(p, ticks);
    float epsilon = 1.0e-5f;
    float dsum = 0.0f;
    float dsumerr = 0.0f;
    float tmp;
    vec4 s;
    for (int i = 0; i < 512; i++) {
        if (dsum > 16.0f) {
            memcpy(s.v, p1.v, sizeof(vec3));
            s.v[3] = 0.0f;
            return s;
        }
#if ! WOOPWOOP
        float old_dsum = dsum;
        float old_dsumerr = dsumerr;
#endif
        tmp = dsum;
        dsum = dsum + d;
        tmp = tmp - (dsum - d);
        dsumerr = dsumerr + tmp;
        p1 = addv3(addv3(
                    mulv3(r, mkv3(dsumerr, dsumerr, dsumerr)),
                    mulv3(r, mkv3(dsum, dsum, dsum))),
                p);
        d = dist_object(p1, ticks);
        if (d < epsilon) {
#if ! WOOPWOOP
            dsum = old_dsum;
            dsumerr = old_dsumerr;
#endif
            break;
        }
    }
    if (d > epsilon) {
        memcpy(s.v, p1.v, sizeof(vec3));
        s.v[3] = 0.0f;
        return s;
    }
#if WOOPWOOP
	for (int i = 0; i < 16 && d < 0.0f; i++) {
        tmp = dsum;
        dsum = dsum + d;
        tmp = tmp - (dsum - d);
        dsumerr = dsumerr + tmp;
        p1 = addv3(addv3(
                    mulv3(r, mkv3(dsumerr, dsumerr, dsumerr)),
                    mulv3(r, mkv3(dsum, dsum, dsum))),
                p);
        d = dist_object(p1, ticks);
	}
#endif
    p1 = addv3(addv3(
                mulv3(r, mkv3(dsumerr, dsumerr, dsumerr)),
                mulv3(r, mkv3(dsum, dsum, dsum))),
            p);
    memcpy(s.v, p1.v, sizeof(vec3));
    s.v[3] = 1.0f;
    return s;
}

vec3 light1;
vec3 light2;
vec3 shade(vec3 p, uint64_t ticks) {
    (void)ticks;
    vec3 n = grad(p, ticks);
    vec4 m1 = trace(subv3(p, mulv3(light1, mkv3(0.05f, 0.05f, 0.05f))), negv3(light1), ticks);
    vec4 m2 = trace(subv3(p, mulv3(light2, mkv3(0.05f, 0.05f, 0.05f))), negv3(light2), ticks);
    float factor1 = (1.0f - m1.v[3]) * dotv3(n, light1);
    float factor2 = (1.0f - m2.v[3]) * dotv3(n, light2);
    vec3 c = mkv3(1.0f, 1.0f, 1.0f);
    float total = (factor1 + factor2) * 0.5f;
    return mulv3(c, mkv3(total, total, total));
}

vec3 gogogo(vec3 p, vec3 ray, uint64_t ticks) {
    vec4 q = trace(p, ray, ticks);
    if (q.v[3] < 1.0f) {
        return mkv3(0x8A / 255.0f, 1.0, 0xC1 / 255.0f);
    }
    memcpy(p.v, q.v, sizeof(p.v));
    return shade(p, ticks);
}

mat3 mkrotmat(vec3 axis, float angle) {
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
            mkv3(v0 * v0 * nc + c     , v1 * v0 * nc + v2 * s, v2 * v0 * nc - v1 * s),
            mkv3(v0 * v1 * nc - v2 * s, v1 * v1 * nc + c     , v2 * v1 * nc + v0 * s),
            mkv3(v0 * v2 * nc + v1 * s, v1 * v2 * nc - v0 * s, v2 * v2 * nc + c     ));
}

const int width = 500;
const int height = 500;

int main(int argc, const char** argv) {
	if (argc < 3) {
		puts("Usage: mushroom <nframes> <outfile>\n");
		exit(1);
	}
	const char* outfile = argv[2];
	FILE* fout;
	if (strncmp(outfile, "-", 2) == 0) {
		fout = stdout;
	} else {
		fout = fopen(outfile, "w");
	}
	if (!fout) {
		puts("Could not open ");
		puts(outfile);
		puts(" for writing\n");
		exit(1);
	}
	light1 = normalizev3(mkv3(-0.5f, -0.2f, -0.1f));
	light2 = normalizev3(mkv3(0.1f, -0.1f, -1.0f));
	int fps = 25;
	int ticks = 0;
	int NFRAMES = atoi(argv[1]);
	float pix_w = 1.0f / width;
	float pix_h = 1.0f / height;
	uint8_t *pict;
	size_t pict_sz = (size_t) (width * height * 3 * sizeof(*pict));
	pict = malloc(pict_sz);
	vec3 axis = normalizev3(mkv3(-0.2f, 1.0f, 0.3f));
	for (int frame = 0; frame < NFRAMES; frame++) {
		ticks = (frame * 1000) / fps;
		uint8_t *pixel = pict;
		fprintf(stderr, "Frame: %d\n", frame);
		for (int y = 0; y < height; y++) for (int x = 0; x < height; x++) {
			float xf = ((float)(x * 2) / (float)width + pix_w) - 1.0f;
			float yf = ((float)(y * 2) / (float)height + pix_h) - 1.0f;
			float angle = -TAU * ((ticks % 15000) / 14999.0f);
			mat3 rotmat = mkrotmat(axis, angle);
			vec3 p = mkv3(0.0f, 0.0f, 3.0f);
			vec3 t = mkv3(xf, yf, 2.0f);
			p = mulm3v3(rotmat, p);
			t = mulm3v3(rotmat, t);
			vec3 ray = normalizev3(subv3(t, p));
			vec3 color = gogogo(p, ray, ticks);
			pixel[0] = (uint8_t) (clampf(color.v[0], 0.0f, 1.0f) * 255.0);
			pixel[1] = (uint8_t) (clampf(color.v[1], 0.0f, 1.0f) * 255.0);
			pixel[2] = (uint8_t) (clampf(color.v[2], 0.0f, 1.0f) * 255.0);
			pixel += 3;
		}
		fwrite(pict, pict_sz, 1, fout);
		fflush(fout);
	}
	free(pict);
	fclose(fout);
	return 0;
}

