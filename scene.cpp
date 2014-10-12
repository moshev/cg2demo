#include <stdlib.h>
#include <stdio.h>
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

static inline uint8_t consume1(const char **scene, size_t *scenesz) {
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

static inline int parse_num(const char **scene, size_t *scenesz, char **shader, size_t *shadersz) {
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

static int parse_df(const char **scene, size_t *scenesz, char **shader, size_t *shadersz);
static int parse_gf(const char **scene, size_t *scenesz, char **shader, size_t *shadersz);
static int parse_vf(const char **scene, size_t *scenesz, char **shader, size_t *shadersz);


static int parse_df(const char **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for DF\n");
        return 0;
    }
    enum distance_func df = (enum distance_func) CONSUME1;
    switch (df) {
    case DF_CUBE:
        APPENDSTR("cube(p, ");
        PARSE_VF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case DF_CUBE3:
        APPENDSTR("cube(p, ");
        PARSE_VF;
        APPENDSTR(", ");
        PARSE_VF;
        APPENDSTR(")");
        return 1;
    case DF_CYLINDER_CAP:
        APPENDSTR("cylinder_caps(p, ");
        PARSE_VF;
        APPENDSTR(", ");
        PARSE_VF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case DF_TORUS:
        APPENDSTR("torus(p, ");
        PARSE_VF;
        APPENDSTR(", normalize(");
        PARSE_VF;
        APPENDSTR("), ");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case DF_PLANE:
        APPENDSTR("plane(p, ");
        PARSE_VF;
        APPENDSTR(", normalize(");
        PARSE_VF;
        APPENDSTR("))");
        return 1;
    case DF_SPHERE:
        APPENDSTR("sphere(p, ");
        PARSE_VF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case DF_MAX:
        APPENDSTR("max(");
        PARSE_DF;
        APPENDSTR(", ");
        PARSE_DF;
        APPENDSTR(")");
        return 1;
    case DF_MIN:
        APPENDSTR("min(");
        PARSE_DF;
        APPENDSTR(", ");
        PARSE_DF;
        APPENDSTR(")");
        return 1;
    case DF_MIX:
        APPENDSTR("mixfix(");
        PARSE_DF;
        APPENDSTR(", ");
        PARSE_DF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    }
    LOGF("Unknown DF: %c\n", df);
    return 0;
}

static int parse_gf(const char **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for GF\n");
        return 0;
    }
    enum generic_func gf = (enum generic_func) CONSUME1;
    switch (gf) {
    case GF_CLAMP:
        APPENDSTR("clamp(");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case GF_MAX:
        APPENDSTR("max(");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case GF_MIN:
        APPENDSTR("min(");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case GF_MIX:
        APPENDSTR("mixfix(");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case GF_NUMBER:
        PARSE_NUM;
        return 1;
    case GF_SMOOTH:
        APPENDSTR("smoothstep(");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    case GF_TIME:
        APPENDSTR("timing(int(");
        PARSE_GF;
        APPENDSTR(" * 1000))");
        return 1;
    case GF_TIME2:
        APPENDSTR("timing2(int(");
        PARSE_GF;
        APPENDSTR(" * 1000))");
        return 1;
    }
    LOGF("Unknown GF: %c\n", gf);
    return 0;
}

static int parse_vf(const char **scene, size_t *scenesz, char **shader, size_t *shadersz) {
    if (!scenesz || !*scenesz) {
        LOG("No bytes left for VF\n");
        return 0;
    }
    enum vec_func vf = (enum vec_func) CONSUME1;
    switch (vf) {
    case VF_VECTOR:
        APPENDSTR("vec3(");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(", ");
        PARSE_GF;
        APPENDSTR(")");
        return 1;
    }
    LOGF("Unknown VF: %c\n", vf);
    return 0;
}

static const char prologue[] = "float dist_object(vec3 p) { return ";
static const char epilogue[] = " ; }";

int parse_scene(const char *scene, size_t scenesz, char **shader, size_t *shadersz) {
    size_t needed = sizeof(prologue) + sizeof(epilogue) - 2;
    char *parsed = nullptr;
    const char *scene_in = scene;
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
