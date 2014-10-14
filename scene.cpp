#include <stdint.h>
#include <string.h>

#include "cg2demo.h"
#include "scene.h"

/*
Actual parsing functions.
The pointers will be moved!
if shader is nullptr, nothing is written, but shadersz is still updated.
Use that to validate input and calculate size.
*/

static inline uint8_t consume1(const uint8_t **scene, size_t *scenesz) {
    (*scenesz)--;
    return (uint8_t)*(*scene)++;
}

#define CONSUME1 consume1(scene, scenesz)

static inline void appendstr(const char *str, size_t strsz, char **shader, size_t *shadersz) {
    if (shader && *shader) {
        memcpy(*shader, str, strsz);
        *shader += strsz;
    }
    *shadersz += strsz;
}

#define APPENDSTR(str) appendstr(str, sizeof(str) - 1, shader, shadersz)

static inline int parse_num(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (*scenesz < 2) {
        LOG("No bytes left for NUM\n");
        return 0;
    }
    uint16_t x = 0;
    x |= ((uint16_t)CONSUME1) << 8;
    x |= ((uint16_t)CONSUME1) & 0xFF;
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

#define PARSE_NUM parse_num(scene, scenesz, shader, shadersz)

#define PARSE_DF do { if (!parse_df(scene, scenesz, shader, shadersz)) return 0; } while (0)
#define PARSE_GF do { if (!parse_gf(scene, scenesz, shader, shadersz)) return 0; } while (0)
#define PARSE_VF do { if (!parse_vf(scene, scenesz, shader, shadersz)) return 0; } while (0)

static int parse_df(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz);
static int parse_gf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz);
static int parse_vf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz);

static const char _comma[] = ", ";
static const char _pstart[] = "(p";
static const char _cube[] = "cube";
static const char _normalize[] = "normalize";
static const char _lparen[] = "(";
static const char _rparen[] = ")";
static const char _timing[] = "timing";
static const char _int[] = "int";
static const char _multhousand = " * 1000";

static int parse_df(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for DF\n");
        return 0;
    }
    enum distance_func df = (enum distance_func) CONSUME1;
    switch (df) {
    case DF_CUBE:
        APPENDSTR(_cube);
        APPENDSTR(_lparen);
        APPENDSTR(_pstart);
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
        APPENDSTR(_pstart);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_rparen);
        return 1;
    case DF_CYLINDER_CAP:
        APPENDSTR("cylinder_caps");
        APPENDSTR(_lparen);
        APPENDSTR(_pstart);
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
        APPENDSTR(_pstart);
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
        APPENDSTR(_pstart);
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
        APPENDSTR(_pstart);
        APPENDSTR(_comma);
        PARSE_VF;
        APPENDSTR(_comma);
        PARSE_GF;
        APPENDSTR(_rparen);
        return 1;
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
    LOGF("Unknown DF: %c\n", df);
    return 0;
}

static int parse_gf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for GF\n");
        return 0;
    }
    enum generic_func gf = (enum generic_func) CONSUME1;
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
    case GF_NUMBER:
        PARSE_NUM;
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
    }
    LOGF("Unknown GF: %c\n", gf);
    return 0;
}

static int parse_vf(const uint8_t **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for VF\n");
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
    LOGF("Unknown VF: %c\n", vf);
    return 0;
}

static const char prologue[] = "float dist_object(vec3 p){return ";
static const char epilogue[] = ";}";

int parse_scene(const uint8_t *scene, size_t scenesz, char **shader, size_t *shadersz) {
    size_t needed = sizeof(prologue) + sizeof(epilogue) - 2;
    char *parsed = nullptr;
    const uint8_t *scene_in = scene;
    size_t scenesz_in = scenesz;
    if (!parse_df(&scene_in, &scenesz_in, nullptr, &needed)) {
        LOG("error parsing scene\n");
        return 0;
    }
    scene_in = scene;
    scenesz_in = scenesz;
    parsed = (char *)malloc(needed);
    if (!parsed) {
        LOGF("error malloc: %s\n", strerror(errno));
        return 0;
    }
    needed = sizeof(prologue) + sizeof(epilogue) - 2;
    memcpy(parsed, prologue, sizeof(prologue) - 1);
    char *parsed_in = parsed + sizeof(prologue) - 1;
    if (!parse_df(&scene_in, &scenesz_in, &parsed_in, &needed)) {
        LOG("error parsing scene\n");
        return 0;
    }
    memcpy(parsed_in, epilogue, sizeof(epilogue) - 1);
    *shader = parsed;
    *shadersz = needed;
    return 1;
}
