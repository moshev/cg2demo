#if !defined(CG2DEMO_H)
#define CG2DEMO_H

#if defined(__MACOSX__)
#include <OpenGL/gl3.h>
#else
#include <SDL_opengl.h>
#endif

#include <stdio.h>

#if defined(NDEBUG)
#define LOG(str)
#define LOGF(fmt, ...)
#else
#define LOGF(fmt, ...) fprintf(flog, "[FILE %s LINE %d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); fflush(flog)
#define LOG(str) LOGF("%s", str)
#endif

static const int MOTIONBLUR_FACTOR = 3;

extern FILE *flog;
int read_file(const char *path, char **text, size_t *sz);

#include "protodecl.inc"

#endif
