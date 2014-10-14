#if defined(NDEBUG)
#include "shaders.inc"
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#include "cg2demo.h"

int get_vertex_shader(const char **vs, size_t *vssz) {
#if defined(NDEBUG)
    *vs = vertex_glsl;
    *vssz = vertex_glslsz;
    return 1;
#else
    char *source;
    int result = read_file("vertex.glsl", &source, vssz);
    if (result) {
        *vs = source;
    }
    return result;
#endif
}

int get_fragment_shader_pre(const char **fspre, size_t *fspresz) {
#if defined(NDEBUG)
    *fspre = fragment_pre_glsl;
    *fspresz = fragment_pre_glslsz;
    return 1;
#else
    char *source;
    int result = read_file("fragment_pre.glsl", &source, fspresz);
    if (result) {
        *fspre = source;
    }
    return result;
#endif
}

int get_fragment_shader_post(const char **fspost, size_t *fspostsz) {
#if defined(NDEBUG)
    *fspost = fragment_post_glsl;
    *fspostsz = fragment_post_glslsz;
    return 1;
#else
    char *source;
    int result = read_file("fragment_post.glsl", &source, fspostsz);
    if (result) {
        *fspost = source;
    }
    return result;
#endif
}

