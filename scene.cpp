#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "cg2demo.h"
#include "scene.h"
#include "math3d.h"

/*
Actual parsing functions.
The pointers will be moved!
if shader is nullptr, nothing is written, but shadersz is still updated.
Use that to validate input and calculate size.
*/

static inline uint8_t consume1(const uint8_t **scene, size_t *scenesz) {
    (*scenesz)--;
    return *(*scene)++;
}

#define CONSUME1 consume1(scene, scenesz)

static inline void appendstr(const char *str, size_t strsz, char **shader, size_t *shadersz) {
    if (shader && *shader) {
        memcpy(*shader, str, strsz);
        *shader += strsz;
    }
    *shadersz += strsz;
}

#define APPENDSTR(str) appendstr(str, strlen(str), shader, shadersz)

static inline int parse_num(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (*scenesz < 2) {
        LOG("No bytes left for NUM");
        return 0;
    }
    uint16_t x = 0;
    x |= ((uint16_t)CONSUME1) << 8;
    x |= ((uint16_t)CONSUME1) & 0xFF;
    // have to clear high bit if not set
    // the number is technically a 15-bit number
    if (!(x & 0x4000)) {
        x = x & 0x7fff;
    }
    float f = (float)(int16_t)x;
    f /= 256.0;
    // haha, no snprintf and no ftoa in MSVS
    // sure, why not
    char strf[1024];
    size_t strfsz;
    strfsz = sprintf(strf, "%f", f);
    appendstr(strf, strfsz, shader, shadersz);
    return 1;
}

#define PARSE_NUM do { if (!parse_num(scene, scenesz, shader, shadersz)) return 0; } while (0)

#define PARSE_DF do { if (!parse_df(scene, scenesz, shader, shadersz)) return 0; } while (0)
#define PARSE_GF do { if (!parse_gf(scene, scenesz, shader, shadersz)) return 0; } while (0)
#define PARSE_VF do { if (!parse_vf(scene, scenesz, shader, shadersz)) return 0; } while (0)

static int parse_df(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz);
static int parse_gf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz);
static int parse_vf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz);

static const char _comma[] = ", ";
static const char __p[] = "p";
// used for indirection when tiled
static const char *_p = __p;
static const char _dot[] = ".";
static const char _cube[] = "cube";
static const char _tile[] = "tile";
static const char _normalize[] = "normalize";
static const char _lparen[] = "(";
static const char _rparen[] = ")";
static const char _timing[] = "timing";
static const char _int[] = "int";
static const char _multhousand[] = " * 1000";

static int parse_df(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for DF");
        return 0;
    }
    enum distance_func df = (enum distance_func) CONSUME1;
    switch (df) {
    case DF_CUBE:
        APPENDSTR(_cube);
        APPENDSTR(_lparen);
        APPENDSTR(_p);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        APPENDSTR("float");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_rparen);
        APPENDSTR(_rparen);
        return 1;
    case DF_CUBE3:
        APPENDSTR(_cube);
        APPENDSTR(_lparen);
        APPENDSTR(_p);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_rparen);
        return 1;
    case DF_CYLINDER_CAP:
        APPENDSTR("cylinder_caps");
        APPENDSTR(_lparen);
        APPENDSTR(_p);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case DF_TORUS:
        APPENDSTR("torus");
        APPENDSTR(_lparen);
        APPENDSTR(_p);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        APPENDSTR(_normalize);
        APPENDSTR(_lparen);
        PARSE_VF;
        APPENDSTR(_rparen);
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case DF_PLANE:
        APPENDSTR("plane");
        APPENDSTR(_lparen);
        APPENDSTR(_p);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        APPENDSTR(_normalize);
        APPENDSTR(_lparen);
        PARSE_VF;
        APPENDSTR("))");
        return 1;
    case DF_SPHERE:
        APPENDSTR("sphere");
        APPENDSTR(_lparen);
        APPENDSTR(_p);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case DF_TILED:
    {
        // have to change _p to
        // tile(_p, VF)
        // and call parse_df
        const char *oldp = _p;
        char *tiledp;
        size_t tiledpsz;
        const uint8_t *scene_in = *scene;
        size_t scenesz_in = *scenesz;
        tiledpsz = 0;
        if (!parse_vf(&scene_in, &scenesz_in, nullptr, &tiledpsz)) {
            return 0;
        }
        tiledpsz = strlen(_tile) + strlen(_lparen) + strlen(oldp) + strlen(_comma) + tiledpsz + strlen(_rparen);
        tiledp = (char *)malloc(tiledpsz + 1);
        if (!tiledp) {
            LOG("Error malloc");
            return 0;
        }
        _p = tiledp;
        const char *parts[] = {_tile, _lparen, oldp, _comma, nullptr};
        tiledpsz = 0;
        for (int i = 0; parts[i]; i++) {
            memcpy(tiledp + tiledpsz, parts[i], strlen(parts[i]));
            tiledpsz += strlen(parts[i]);
        }
        char *tiledp_in = tiledp + tiledpsz;
        if (!parse_vf(scene, scenesz, &tiledp_in, &tiledpsz)) {
            free(tiledp);
            return 0;
        }
        memcpy(tiledp + tiledpsz, _rparen, strlen(_rparen));
        tiledpsz += strlen(_rparen);
        tiledp[tiledpsz] = '\0';
        LOGF("tiled: %s", _p);
        if (!parse_df(scene, scenesz, shader, shadersz)) {
            free(tiledp);
            return 0;
        }
        _p = oldp;
        free(tiledp);
        return 1;
    }
    case DF_MAX:
        APPENDSTR("max");
        APPENDSTR(_lparen);
        PARSE_DF;
        APPENDSTR(_comma);
        PARSE_DF;
        APPENDSTR(_rparen);
        return 1;
    case DF_MIN:
        APPENDSTR("min");
        APPENDSTR(_lparen);
        PARSE_DF;
        APPENDSTR(_comma);
        PARSE_DF;
        APPENDSTR(_rparen);
        return 1;
    case DF_MIX:
        APPENDSTR("mixfix");
        APPENDSTR(_lparen);
        PARSE_DF;
        APPENDSTR(_comma);
        PARSE_DF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    }
    LOGF("Unknown DF: %c", df);
    return 0;
}

static int parse_gf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for GF");
        return 0;
    }
    enum generic_func gf = (enum generic_func) CONSUME1;
    if (GF_NUMBER) {
        // put back
        (*scene)--;
        (*scenesz)++;
        PARSE_NUM;
        return 1;
    }
    switch (gf) {
    case GF_CLAMP:
        APPENDSTR("clamp");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case GF_MAX:
        APPENDSTR("max");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case GF_MIN:
        APPENDSTR("min");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case GF_MIX:
        APPENDSTR("mixfix");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case GF_SMOOTH:
        APPENDSTR("smoothstep");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    case GF_TIME:
        APPENDSTR(_timing);
        APPENDSTR(_lparen);
        APPENDSTR(_int);
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_multhousand);
        APPENDSTR(_rparen);
        APPENDSTR(_rparen);
        return 1;
    case GF_TIME2:
        APPENDSTR(_timing);
        APPENDSTR("2");
        APPENDSTR(_lparen);
        APPENDSTR(_int);
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_multhousand);
        APPENDSTR(_rparen);
        APPENDSTR(_rparen);
        return 1;
    case GF_PDOTX:
        APPENDSTR(_p);
        APPENDSTR(_dot);
        APPENDSTR("x");
        return 1;
    case GF_PDOTY:
        APPENDSTR(_p);
        APPENDSTR(_dot);
        APPENDSTR("y");
        return 1;
    case GF_PDOTZ:
        APPENDSTR(_p);
        APPENDSTR(_dot);
        APPENDSTR("z");
        return 1;
    }
    LOGF("Unknown GF: %c", gf);
    return 0;
}

static int parse_vf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for VF");
        return 0;
    }
    enum vec_func vf = (enum vec_func) CONSUME1;
    switch (vf) {
    case VF_VECTOR:
        APPENDSTR("vec3");
        APPENDSTR(_lparen);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
    }
    LOGF("Unknown VF: %c", vf);
    return 0;
}

static const char prologue[] = "float dist_object(vec3 p){return ";
static const char epilogue[] = ";}";

size_t parse_scene(const uint8_t *scene, size_t scenesz, char **shader, size_t *shadersz) {
    size_t needed = sizeof(prologue) + sizeof(epilogue) - 2;
    char *parsed = nullptr;
    const uint8_t *scene_in = scene;
    size_t scenesz_in = scenesz;
    if (!parse_df(&scene_in, &scenesz_in, nullptr, &needed)) {
        LOG("error parsing scene");
        return 0;
    }
    if (!shader) {
        *shadersz = needed;
        return scene - scene_in;
    }
    scene_in = scene;
    scenesz_in = scenesz;
    parsed = (char *)malloc(needed);
    if (!parsed) {
        LOGF("error malloc: %s", strerror(errno));
        return 0;
    }
    needed = sizeof(prologue) + sizeof(epilogue) - 2;
    memcpy(parsed, prologue, sizeof(prologue) - 1);
    char *parsed_in = parsed + sizeof(prologue) - 1;
    if (!parse_df(&scene_in, &scenesz_in, &parsed_in, &needed)) {
        LOG("error parsing scene");
        return 0;
    }
    memcpy(parsed_in, epilogue, sizeof(epilogue) - 1);
    *shader = parsed;
    *shadersz = needed;
    return scene - scene_in;
}

int split_scenes(const uint8_t *scenes, size_t scenessz, struct scene **parsed, size_t *nparsed) {
    const uint8_t *scene_in = scenes;
    size_t scenesz_in = scenessz;
    size_t n = 0;
    struct scene *p;
    size_t tmp;
    while (scenesz_in > 8) {
        scenesz_in -= 8;
        scene_in += 8;
        if (!parse_df(&scene_in, &scenesz_in, nullptr, &tmp)) {
            LOGF("Couldn't parse scene %d", (int)n);
            return 0;
        }
        n++;
    }
    p = (struct scene *)malloc(n * sizeof(struct scene));
    if (!p) {
        LOG("error malloc");
        return 0;
    }
    scene_in = scenes;
    scenesz_in = scenessz;
    n = 0;
    while (scenesz_in > 8) {
        float tx = (int16_t)(scene_in[0] * 256 + scene_in[1]) / 256.0f;
        float ty = (int16_t)(scene_in[2] * 256 + scene_in[3]) / 256.0f;
        float tz = (int16_t)(scene_in[4] * 256 + scene_in[5]) / 256.0f;
        unsigned duration = (unsigned)(scene_in[6] * 256 + scene_in[7]);
        LOGF("Scene %d %f %f %f %d", n, tx, ty, tz, duration);
        p[n].camera_translation = mkv3(tx, ty, tz);
        p[n].duration = duration;
        scenesz_in -= 8;
        scene_in += 8;
        p[n].data = scene_in;
        size_t oldsz = scenesz_in;
        if (!parse_df(&scene_in, &scenesz_in, nullptr, &tmp)) {
            LOGF("Couldn't parse scene %d", (int)n);
            free(p);
            return 0;
        }
        p[n].datasz = oldsz - scenesz_in;
        ++n;
    }
    *parsed = p;
    *nparsed = n;
    return 1;
}
