#if defined(NDEBUG)
#include "shaders.inc"
#else
#include <stdlib.h>
#include <stdio.h>
#endif
#include <string.h>
#include <malloc.h>

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
    int result;
    char *source;
#if defined(NDEBUG)
    source = (char *)malloc(fragment_pre_glslsz);
    if (!source) {
        LOG("error malloc");
        return 0;
    }
    memcpy(source, fragment_pre_glsl, fragment_pre_glslsz);
    *fspresz = fragment_pre_glslsz;
    result = 1;
#else
    result = read_file("fragment_pre.glsl", &source, fspresz);
#endif
    if (result) {
        char *motionblur_constant = source;
        // yes the space at the end is important
        const char name[] = "MOTIONBLUR_FACTOR ";
        char factor[16];
        for (; motionblur_constant < source + *fspresz - sizeof(name) - sizeof(factor); ++motionblur_constant) {
            if (memcmp(motionblur_constant, name, sizeof(name) - 1) == 0) {
                int printed = sprintf(factor, "%d", MOTIONBLUR_FACTOR);
                if (printed > 0) {
                    memcpy(motionblur_constant + sizeof(name) - 1, factor, printed);
                }
                break;
            }
        }
        *fspre = source;
    }
    return result;
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

int get_fragment_shader_text(const char **fstext, size_t *fstextsz) {
#if defined(NDEBUG)
    *fstext = fragment_text_glsl;
    *fstextsz = fragment_text_glslsz;
    return 1;
#else
    char *source;
    int result = read_file("fragment_text.glsl", &source, fstextsz);
    if (result) {
        *fstext = source;
    }
    return result;
#endif
}

