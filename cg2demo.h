#if !defined(CG2DEMO_H)
#define CG2DEMO_H

#include <SDL_opengl.h>
#include <stdio.h>

#if defined(_DEBUG)
#define LOG(str) fputs(str, flog)
#define LOGF(fmt, ...) fprintf(flog, fmt, __VA_ARGS__)
#else
#define LOG(str) (void)
#define LOGF(fmt, ...) (void)
#endif

extern FILE *flog;

#include "protodecl.inc"

#endif
