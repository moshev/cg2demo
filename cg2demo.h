#if !defined(CG2DEMO_H)
#define CG2DEMO_H

#include <SDL_opengl.h>
#include <stdio.h>

#if defined(_DEBUG)
#define LOG(str) fputs(str, flog); fflush(flog)
#define LOGF(fmt, ...) fprintf(flog, fmt, __VA_ARGS__); fflush(flog)
#else
#define LOG(str)
#define LOGF(fmt, ...)
#endif

extern FILE *flog;
int read_file(const char *path, char **text, size_t *sz);

#include "protodecl.inc"

#endif
