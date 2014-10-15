#if !defined(CG2DEMO_SCENE_H)
#define CG2DEMO_SCENE_H

#include "math3d_t.h"

enum distance_func {
    // cube, centre, radius
    DF_CUBE = 'C',
    // cube, centre, vec3 radiuses
    DF_CUBE3 = 'D',
    // sphere, centre, radius
    DF_SPHERE = 'S',
    // cylinder, centreA, centreB, radius
    DF_CYLINDER_CAP = 'I',
    // torus, centre, normal, radius to ring, ring radius
    DF_TORUS = 'T',
    // plane, point, normal
    DF_PLANE = 'P',
    // tiled
    DF_TILED = '#',
    // mix, distance_func, distance_func, generic_func
    DF_MIX = 'X',
    // min, distance_func, distance_func
    DF_MIN = 'm',
    // max, distance_func, distance_func
    DF_MAX = 'M',
};

enum generic_func {
    // literal number
    GF_NUMBER = 'F',
    // time, period
    GF_TIME = 'T',
    // time2 (oscillating), period
    GF_TIME2 = 'U',
    // smoothstep, min, max, arg
    GF_SMOOTH = 'H',
    // clamp, min, max, arg
    GF_CLAMP = 'C',
    // mix, generic_func, generic_func, generic_func
    GF_MIX = DF_MIX,
    // min, generic_func, generic_func
    GF_MIN = DF_MIN,
    // max, generic_func, generic_func
    GF_MAX = DF_MAX,
    // p.x
    GF_PDOTX = '0',
    // p.y
    GF_PDOTY = '1',
    // p.z
    GF_PDOTZ = '2',
};

enum vec_func {
    // literal vector
    VF_VECTOR = 'V',
};

struct scene {
    vec3 camera_translation;
    unsigned duration;
    const uint8_t *data;
    size_t datasz;
};

/* return number of bytes consumed, 0 on error */
size_t parse_scene(const uint8_t *scene, size_t scenesz, char **shader, size_t *shadersz);

int split_scenes(const uint8_t *scenes, size_t scenessz, struct scene **parsed, size_t *nparsed);

#endif

