#if !defined(CG2DEMO_H)
#define CG2DEMO_H

#include <SDL_opengl.h>
#include <stdio.h>

#if defined(NDEBUG)
#define LOG(str)
#define LOGF(fmt, ...)
#else
#define LOGF(fmt, ...) fprintf(flog, "[FILE %s LINE %d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); fflush(flog)
#define LOG(str) LOGF("%s", str)
#endif

extern FILE *flog;
int read_file(const char *path, char **text, size_t *sz);

#include "protodecl.inc"

#endif
