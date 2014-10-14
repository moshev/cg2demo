#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "cg2demo.h"
#include "math3d.h"
#include "scene.h"
#include "scenedsl.h"
#include "shaders.h"

#include "protodef.inc"

FILE *flog;

/* read file into memory, return 1 on success, 0 on failure */
int read_file(const char *path, char **text, size_t *sz);

static const unsigned int WIDTH = 1024;
static const unsigned int HEIGHT = 768;

/* rectangle */
static GLfloat RECTANGLE[] = {
    -1, -1,
    1, -1,
    1, 1,
    -1, 1
};

/* shader attribute and uniforms */
static GLint attr_p;
static GLint ufrm_width;
static GLint ufrm_height;
static GLint ufrm_millis;
static GLint ufrm_camera;

static uint8_t *taudigits;
static const size_t ntaudigits = 1280;

static const char *vertex_glsl;
static size_t vertex_glslsz;
static const char *fragment_pre_glsl;
static size_t fragment_pre_glslsz;
static const char *fragment_post_glsl;
static size_t fragment_post_glslsz;

static int renderloop(SDL_Window *window, SDL_GLContext context);

/* exits with error status and logs error if program wasn't successfully linked */
static void check_program(GLuint id);

/* exits with error status and logs error if shader wasn't successfully compiled */
static void check_shader(GLuint id);

static GLuint create_program_from_files(const char *vspath, const char *fspath);
static GLuint create_program(const char *vs, size_t szvs, const char *fs, size_t szfs);

// http://en.wikipedia.org/wiki/Modular_exponentiation#Right-to-left_binary_method
static int modpow(int base, int exponent, int modulus) {
    int result = 1;
    base %= modulus;
    while (exponent > 0) {
        if (exponent & 1) result = (result * base) % modulus;
        exponent >>= 1;
        base = (base * base) % modulus;
    }
    return result;
}

// I could not understand Bailey-Borwein-Plouffe
// this is not quite that formula
// but it generates some randomish numbers
static int S(int j, int n) {
    double s = 0;
    for (int k = 0; k <= n; ++k) {
        int r = 8 * k + j;
        s += (2 * modpow(16, n - k, r)) / (double)r;
    }
    return (int)s & 0xf;
}

static int tau(int n) {
    int r = (4 * S(1, n) - 2 * S(2, n) - S(3, n) - S(4, n)) & 0xf;
    return r;
}

struct audio_state {
    unsigned samples;
    int note;
};

static void audio_callback(void *userdata, Uint8 *stream, int len);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
#if defined(NDEBUG)
    flog = nullptr;
#else
    flog = fopen("flog.txt", "w");
    if (!flog) {
        exit(-1);
    }
#endif
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        LOG("FAILED TO INIT VIDEO");
        exit(1);
    }
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        LOG("FAILED TO INIT AUDIO");
        exit(1);
    }
    audio_state as;
    as.samples = 0;
    as.note = 0;
    SDL_AudioSpec desired;
    desired.channels = 1;
    desired.format = AUDIO_S16;
    desired.freq = 44100;
    desired.samples = 4096;
    desired.callback = audio_callback;
    desired.userdata = &as;
    SDL_OpenAudio(&desired, nullptr);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_Window *window = SDL_CreateWindow("Super Special Awesome Demo 2014",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    if (!window) {
        LOGF("Could not create window: %s", SDL_GetError());
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

#include "protoget.inc"

    if (!get_vertex_shader(&vertex_glsl, &vertex_glslsz)) {
        LOG("error reading vertex shader source");
        exit(1);
    }
    if (!get_fragment_shader_pre(&fragment_pre_glsl, &fragment_pre_glslsz)) {
        LOG("error reading fragment shader pre-scene part source");
        exit(1);
    }
    if (!get_fragment_shader_post(&fragment_post_glsl, &fragment_post_glslsz)) {
        LOG("error reading fragment shader post-scene part source");
        exit(1);
    }

    taudigits = (uint8_t *)malloc(ntaudigits);
    // gen tau
    for (size_t i = 0; i < ntaudigits; i++) {
        taudigits[i] = tau((int)i);
    }
    SDL_PauseAudio(0);

    // main loop
    int result = renderloop(window, context);
    SDL_DestroyWindow(window);
    return result;
}

int read_file(const char *path, char **text, size_t *sz) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        LOGF("Couldn't read %s: %s", path, strerror(errno));
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    LOGF("size %ld", size);
    if (size < 0) {
        LOG("error seeking");
        fclose(f);
        return 0;
    }
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size);
    if (!buf) {
        LOG("error allocating memory");
        fclose(f);
        return 0;
    }
    size_t read = 0;
    while ((size_t)size > read) {
        LOGF("read %d", (int)read);
        size_t res = fread(buf + read, 1, (size_t)size - read, f);
        LOGF("res %d", (int)res);
        if (res <= 0) {
            LOGF("error reading: %s", strerror(errno));
            fclose(f);
            free(buf);
            return 0;
        }
        read += res;
    }
    fclose(f);
    *text = buf;
    *sz = (size);
    return 1;
}

static GLuint create_program_from_files(const char *vspath, const char *fspath) {
    char *vs, *fs;
    size_t szvs, szfs;
    if (!read_file(vspath, &vs, &szvs)) {
        LOG("error reading vertex shader source");
        exit(1);
    }
    if (!read_file(fspath, &fs, &szfs)) {
        LOG("error reading fragment shader source");
        exit(1);
    }
    GLuint prog = create_program(vs, szvs, fs, szfs);
    free(vs);
    free(fs);
    return prog;
}

static GLuint create_program_from_scene(const uint8_t *scene, size_t scenesz) {
    char *fsscene;
    size_t fsscenesz;
    if (!parse_scene(scene, scenesz, &fsscene, &fsscenesz)) {
        LOG("Error parsing scene");
        exit(1);
    }
    char *fs;
    size_t fssz = fragment_pre_glslsz + fsscenesz + fragment_post_glslsz + 1;
    fs = (char *)malloc(fssz);
    if (!fs) {
        LOG("Error malloc");
        exit(1);
    }
    memcpy(fs, fragment_pre_glsl, fragment_pre_glslsz);
    memcpy(fs + fragment_pre_glslsz, fsscene, fsscenesz);
    memcpy(fs + fragment_pre_glslsz + fsscenesz, fragment_post_glsl, fragment_post_glslsz);
    fs[fssz - 1] = '\0';
    LOGF("generated shader:\n-----\n%.*s\n------\n", (int)fssz, fs);

    GLuint prog = create_program(vertex_glsl, vertex_glslsz, fs, fssz);
    free(fsscene);
    return prog;
}

static void check_shader(GLuint shaderid) {
    GLint compiled;
    glGetShaderiv(shaderid, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        LOG("Compilation error");
        GLsizei length;
        glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &length);
        if (length) {
            char *infolog = (char *)malloc(length);
            if (!infolog) {
                LOG("error malloc");
                exit(1);
            }
            glGetShaderInfoLog(shaderid, length, &length, infolog);
            LOGF("shader compile log:\n%.*s", (int)length, infolog);
            exit(1);
        }
    }
}

static void check_program(GLuint progid) {
    GLint compiled;
    glGetProgramiv(progid, GL_LINK_STATUS, &compiled);
    if (!compiled) {
        LOG("Link error");
        GLsizei length;
        glGetProgramiv(progid, GL_INFO_LOG_LENGTH, &length);
        if (length) {
            char *infolog = (char *)malloc(length);
            if (!infolog) {
                LOG("error malloc");
                exit(1);
            }
            glGetProgramInfoLog(progid, length, &length, infolog);
            LOGF("program link log:\n%.*s", (int)length, infolog);
            exit(1);
        }
    }
}

static GLuint create_shader(GLenum shader_type, const char *src, size_t srcsz) {
    if (shader_type != GL_VERTEX_SHADER && shader_type != GL_FRAGMENT_SHADER) {
        LOGF("unknown shader type: %d", (int)shader_type);
        exit(1);
    }
    GLuint shaderid = glCreateShader(shader_type);
    GLint isz = (GLint)srcsz;
    glShaderSource(shaderid, 1, &src, &isz);
    glCompileShader(shaderid);
    check_shader(shaderid);
    return shaderid;
}

static GLuint create_program(const char *vs, size_t szvs, const char *fs, size_t szfs) {
    GLuint prog = glCreateProgram();
    GLuint vsid = create_shader(GL_VERTEX_SHADER, vs, szvs);
    GLuint fsid = create_shader(GL_FRAGMENT_SHADER, fs, szfs);
    glAttachShader(prog, vsid);
    glAttachShader(prog, fsid);
    glLinkProgram(prog);
    check_program(prog);
    attr_p = glGetAttribLocation(prog, "p");
    ufrm_width = glGetUniformLocation(prog, "width");
    ufrm_height = glGetUniformLocation(prog, "height");
    ufrm_millis = glGetUniformLocation(prog, "millis");
    ufrm_camera = glGetUniformLocation(prog, "camera");
    return prog;
}

static float smootherstep(float x) {
    return  x*x*x*(x*(x * 6 - 15) + 10);
}

static const uint8_t SCENES[] = {
SCENESPEC(0, 0, 0, 30,
SC_MIN(
  SC_SPHERE(SC_VEC3(SC_FIXED(-0.25), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.25)),
  SC_SPHERE(SC_VEC3(SC_FIXED(0.25), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.25))
)),

SCENESPEC(-0.25, 0, 0, 20,
SC_MIN(
  SC_MIX(
    SC_SPHERE(SC_VEC3(SC_TIME2(SC_FIXED(3)), SC_FIXED(0), SC_FIXED(-0.0)), SC_FIXED(0.2)),
    SC_CUBE(SC_VEC3(SC_FIXED(0.2), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.5)),
    SC_TIME2(SC_FIXED(16))),
  SC_MIN(
//    SC_MIN(
      SC_PLANE(SC_VEC3(SC_FIXED(0), SC_FIXED(-3), SC_FIXED(0)),
               SC_VEC3(SC_FIXED(0), SC_FIXED(1), SC_FIXED(0))),
//      SC_PLANE(SC_VEC3(SC_FIXED(0), SC_FIXED(4), SC_FIXED(0)),
    //           SC_VEC3(SC_FIXED(0), SC_FIXED(-1), SC_FIXED(0)))
    //),
    SC_MIX(
      SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(-2.3)),
               SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
               SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(3))), SC_FIXED(0.25), SC_FIXED(0.6)),
               SC_FIXED(0.2)),
      SC_MIN(SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_FIXED(0.25), SC_FIXED(-2.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2))),
             SC_CUBE3(SC_VEC3(SC_FIXED(0.0), SC_FIXED(0.75), SC_FIXED(-2.3)),
                      SC_VEC3(SC_FIXED(0.25), SC_FIXED(0.25), SC_FIXED(0.2)))),
      SC_TIME2(SC_FIXED(11))
    )
  )
)),

SCENESPEC(0.5, 0, 0, 2,
SC_MIX(
  SC_SPHERE(SC_VEC3(SC_TIME2(SC_FIXED(2.4)), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.2)),
  SC_CYLINDER_CAP(SC_VEC3(SC_FIXED(0.1), SC_FIXED(0), SC_FIXED(0)), SC_VEC3(SC_FIXED(0.9), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1)),
  SC_TIME2(SC_FIXED(1.2)),
)),

SCENESPEC(0.5, 0, 0, 2,
SC_MIX(
SC_MIX(SC_TORUS(SC_VEC3(SC_FIXED(-0.1), SC_FIXED(0), SC_FIXED(0)), SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)), SC_FIXED(0.2), SC_FIXED(0.1)),
         SC_SPHERE(SC_VEC3(SC_TIME2(SC_FIXED(4)), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.08)),
         SC_SMOOTH(SC_FIXED(0.03), SC_FIXED(0.4), SC_TIME2(SC_FIXED(4)))),
  SC_CUBE(SC_VEC3(SC_FIXED(1), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.2)),
  SC_SMOOTH(SC_FIXED(0.6), SC_FIXED(1), SC_TIME2(SC_FIXED(4)))
)),

SCENESPEC(0, 0, 0, 2,
SC_MIX(
    SC_TILED(SC_VEC3(SC_FIXED(0.2), SC_FIXED(0.2), SC_FIXED(0.2)),
            SC_CUBE(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)), SC_FIXED(0.1))),
    SC_TORUS(SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(0)),
             SC_VEC3(SC_FIXED(0), SC_FIXED(0), SC_FIXED(1)),
             SC_CLAMP(SC_SMOOTH(SC_FIXED(0.1), SC_FIXED(0.9), SC_TIME2(SC_FIXED(3))), SC_FIXED(0.25), SC_FIXED(0.6)),
             SC_FIXED(0.2)),
    SC_CLAMP(SC_TIME2(SC_FIXED(10)), SC_FIXED(0.05), SC_FIXED(1.0))
))
};

static mat4 mkcamera(Uint32 ticks, mat4 additional) {
    ///*
    float rotf = (ticks % 32000) / 31999.0f;
    float trf = (ticks % 17000) / 16999.0f;
    rotf = smootherstep(rotf);

    //return mkrotationm4(mkv3(0, 1, 0), trf * TAU);
    /*
    return mulm4(
        mulm4(
            additional,
            mkrotationm4(mkv3(0, 1, 0), rotf * TAU)
        ),
        mkrotationm4(normalizev3(mkv3(1.0f + cosf(trf * TAU), 0.5f * sinf(trf * TAU), 0)), trf * TAU)
    );
    */
    //*/
    ///*
    return mkm4identity();
    /*
    mkv4(1, 0, 0, 0),
    mkv4(0, 1, 0, 0),
    mkv4(0, 0, 1, 0),
    mkv4(0, 0, 0, 1));
    */
}

static void switch_scene(GLuint prog, unsigned width, unsigned height) {
    glUseProgram(prog);
    glUniform1i(ufrm_width, width);
    glUniform1i(ufrm_height, height);
}

static int renderloop(SDL_Window *window, SDL_GLContext context) {
    unsigned int width = WIDTH;
    unsigned int height = HEIGHT;
    mat4 camera;

    glViewport(0, 0, width, height);
    //glClearColor(0x8A / 255.0f, 0xFF / 255.0f, 0xC1 / 255.0f, 1);
    glClearColor(0.9f, 0.9f, 0.9f, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    LOG("");
    struct scene *scenes;
    size_t nscenes;
    if (!split_scenes(SCENES, sizeof(SCENES), &scenes, &nscenes)) {
        LOG("error splitting scenes");
        exit(1);
    }
    GLuint *progs = (GLuint *)malloc(nscenes * sizeof(GLuint));
    for (int i = 0; i < nscenes; i++) {
        progs[i] = create_program_from_scene(scenes[i].data, scenes[i].datasz);
    }

    LOGF("total scenes size: %d", (int)sizeof(SCENES));
    
    int scene = 0;
    switch_scene(progs[scene], width, height);

    GLuint vao, buf;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RECTANGLE), RECTANGLE, GL_STATIC_DRAW);
    glEnableVertexAttribArray(attr_p);
    glVertexAttribPointer(attr_p, 2, GL_FLOAT, 0, 0, 0);

    // to keep precise 60fps
    // every third frame will be 17ms
    // instead of 16.
    int waiterror = 0;
    Uint32 ticks_start = SDL_GetTicks();
    const Uint32 ticks_first = ticks_start;
    Uint32 scene_start = ticks_start;
    for (;;) {
        SDL_Event event;
        if ((SDL_GetTicks() - scene_start) / 1000 > scenes[scene].duration) {
            scene = (scene + 1) % nscenes;
            switch_scene(progs[scene], width, height);
            scene_start = SDL_GetTicks();
        }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    width = event.window.data1;
                    height = event.window.data2;
                    glUniform1i(ufrm_width, width);
                    glUniform1i(ufrm_height, height);
                    glViewport(0, 0, width, height);
                }
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1i(ufrm_millis, ticks_start - ticks_first);
        camera = mkcamera(ticks_start - ticks_first, mkm4identity());
        glUniformMatrix4fv(ufrm_camera, 1, 0, &camera.c[0].v[0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        SDL_GL_SwapWindow(window);
        Uint32 allotted = 16;
        if (waiterror == 2) {
            allotted++;
        }
        Uint32 ticks_end = SDL_GetTicks();
        Uint32 frametime = ticks_end - ticks_start;
        if (frametime < allotted) {
            SDL_Delay(allotted - frametime);
            ticks_start += allotted;
        } else {
            ticks_start = SDL_GetTicks();
        }
        waiterror = (waiterror + 1) % 3;
    }
}

// BLUES SCALE
uint16_t noteshz[] = {
    262, 311, 349, 370, 392, 466, 523, 622, 699, 740, 784, 932, 1047, 1245, 1397, 1480
};

//HUNGARIAN MINOR
//uint16_t noteshz[] = {
//    262, 294, 311, 370, 392, 415, 494, 523, 587, 622, 734, 784, 831, 988, 1047, 1175
//};

//AUDIO
void audio_callback(void *userdata, Uint8 *stream, int len) {
    audio_state *as = (audio_state *)userdata;
    Sint16 *buf = (Sint16 *)stream;
    size_t bufsz = len / 2;
    for (size_t i = 0; i < bufsz; i++) {
        int s = as->samples + 1;
        int n = as->note;
        int hz = noteshz[taudigits[n]];
        if ((s * hz) % 44100 < 500 && s > 11025) {
            s = 0;
            n = (n + 1) % ntaudigits;
        }
        hz = noteshz[taudigits[n]];
        double t = ((s * hz) % 44100) / 44099.0;
        buf[i] = (Sint16)(sin(t * TAU) * 0x2000);
        as->samples = s;
        as->note = n;
    }
}
