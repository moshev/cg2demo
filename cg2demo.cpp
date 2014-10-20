#if defined(_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <errno.h>
#include <SDL.h>

#if defined(__MACOSX__)
#include <OpenGL/gl3.h>
#else
#include <SDL_opengl.h>
#endif
#if defined(WIN32)
static void (*glActiveTexture)(GLenum);
#endif

#include <atomic>

#include "cg2demo.h"
#include "math3d.h"
#include "scene.h"
#include "scenedsl.h"
#include "shaders.h"
#include "text.h"
#include "audio.h"


#include "protodef.inc"

FILE *flog;

/* read file into memory, return 1 on success, 0 on failure */
int read_file(const char *path, char **text, size_t *sz);

static bool movie = false;

static FILE *fvideoout = nullptr;
static FILE *faudioout = nullptr;

static const unsigned int WIDTH = 1024;
static const unsigned int HEIGHT = 768;

/* rectangle */
static GLfloat RECTANGLE[] = {
    -1, -1,
    1, -1,
    1, 1,
    -1, 1
};

static const uint8_t SCENES[] = {
#include "scenespecs.inc"
};

static size_t ntaudigits = 1280;

static const char *vertex_glsl;
static size_t vertex_glslsz;
static const char *fragment_pre_glsl;
static size_t fragment_pre_glslsz;
static const char *fragment_post_glsl;
static size_t fragment_post_glslsz;
static const char *fragment_text_glsl;
static size_t fragment_text_glslsz;

static int renderloop(SDL_Window *window, SDL_GLContext context);

/* exits with error status and logs error if program wasn't successfully linked */
static void check_program(GLuint id);

/* exits with error status and logs error if shader wasn't successfully compiled */
static void check_shader(GLuint id);

static GLuint create_program(const char *vs, size_t szvs, const char *fs, size_t szfs);

static void audio_callback(void *userdata, Uint8 *stream, int len);

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
// Shamelessly copied (mostly) from http://www.jasondavies.com/bbp/
static double S(int j, int n) {
    double s = 0;
    int k;
    for (k = 0; k <= n; ++k) {
        int r = 8 * k + j;
        double t = modpow(16, n - k, r);
        s += t / r;
        s -= floor(s);
    }
    k = n + 1;
    double k1 = 0;
    double k2 = 0;
    double f = pow(16.0, -k);
    do {
        k1 = k2;
        k2 = k2 + f / (8.0 * k + j);
        k++;
        f /= 16.0;
    } while (abs(k1 - k2) > 1.0e-128);
    return s + k2;
}

static int tau(int n) {
    int n1 = n - 1;
    double r = (8 * S(1, n1) - 4 * S(2, n1) - 2 * S(3, n1) - 2 * S(4, n1));
    r -= floor(r);
    return (int)floor(16.0 * r);
}

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
    movie = (argc > 1 && strcmp(argv[1], "--record") == 0);
    if (movie) {
        fvideoout = fopen("fvideoout", "w+b");
        faudioout = fopen("faudioout", "w+b");
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        LOG("FAILED TO INIT VIDEO");
        exit(1);
    }
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        LOG("FAILED TO INIT AUDIO");
        exit(1);
    }
    audio_state as;
    memset(&as, 0, sizeof(as));
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
#if defined(WIN32)
    glActiveTexture = (void (*) (GLenum))SDL_GL_GetProcAddress("glActiveTexture");
#endif

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
    if (!get_fragment_shader_text(&fragment_text_glsl, &fragment_text_glslsz)) {
        LOG("error reading fragment shader text-scene source");
        exit(1);
    }


    uint8_t *taudigitsrw = (uint8_t *)malloc(ntaudigits);
    // gen tau
    for (size_t i = 0, j = 0; j < ntaudigits; i++) {
        int digit = tau((int)i + 1);
        if (i < 16) {
            LOGF("tau_%02d = %d", (int)i, digit);
        }
        if ((size_t)digit < nnotesoffsets - 1) {
            taudigitsrw[j++] = (uint8_t)digit;
        }
    }
    as.taudigits = taudigitsrw;
    as.ntaudigits = ntaudigits;

    SDL_MaximizeWindow(window);
    // main loop
    int result = renderloop(window, context);
    SDL_DestroyWindow(window);
    SDL_Quit();
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
    free(fs);
    return prog;
}

static GLuint create_text_program() {
    char *fs;
    size_t fssz = fragment_pre_glslsz + fragment_text_glslsz + 1;
    fs = (char *)malloc(fssz);
    if (!fs) {
        LOG("Error malloc");
        exit(1);
    }
    memcpy(fs, fragment_pre_glsl, fragment_pre_glslsz);
    memcpy(fs + fragment_pre_glslsz, fragment_text_glsl, fragment_text_glslsz);
    fs[fssz - 1] = '\0';
    LOGF("generated shader:\n-----\n%.*s\n------\n", (int)fssz, fs);

    GLuint prog = create_program(vertex_glsl, vertex_glslsz, fs, fssz);
    free(fs);
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
    return prog;
}

static float smootherstep(float x) {
    return  x*x*x*(x*(x * 6 - 15) + 10);
}

static mat4 mkcamera(Uint32 ticks, mat4 additional) {
    ///*
    float rotf = (ticks % 32000) / 31999.0f;
    float trf = (ticks % 17000) / 16999.0f;
    rotf = smootherstep(rotf);

    //return mkrotationm4(mkv3(0, 1, 0), trf * TAU);
    ///*
    return mulm4(
        mulm4(
            additional,
            mkrotationm4(mkv3(0, 1, 0), rotf * TAU)
        ),
        mkrotationm4(normalizev3(mkv3(0.5f + 0.5f * cosf(trf * TAU), 1.0f, 0.5f + 0.5f * sinf(trf * TAU))), trf * TAU)
    );
    //*/
    //*/
    /*
    return mkm4identity();
    /*
    mkv4(1, 0, 0, 0),
    mkv4(0, 1, 0, 0),
    mkv4(0, 0, 1, 0),
    mkv4(0, 0, 0, 1));
    */
}

struct program {
    GLuint id;
    GLint attr_p;
    GLint ufrm_width;
    GLint ufrm_height;
    GLint ufrm_millis;
    GLint ufrm_camera;
    GLint ufrm_currentFramebuffer;
};

static void switch_scene(struct program *prog, unsigned width, unsigned height) {
    glUseProgram(prog->id);
    glUniform1i(prog->ufrm_width, width);
    glUniform1i(prog->ufrm_height, height);
    glEnableVertexAttribArray(prog->attr_p);
    glVertexAttribPointer(prog->attr_p, 2, GL_FLOAT, 0, 0, 0);
}

static int renderloop(SDL_Window *window, SDL_GLContext context) {
    unsigned int width;
    unsigned int height;
    mat4 camera;
    SDL_GetWindowSize(window, (int *)&width, (int *)&height);
    SDL_Event event;
    // in some cases the window is resized by delivering
    // a resize event. try to catch that
    SDL_Delay(100);
    for (int i = 0; SDL_PollEvent(&event) && i < 500; i++) {
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                width = event.window.data1;
                height = event.window.data2;
            }
        }
    }
    LOGF("width: %u", width);
    LOGF("height: %u", height);

    glViewport(0, 0, width, height);
    //glClearColor(0x8A / 255.0f, 0xFF / 255.0f, 0xC1 / 255.0f, 1);
    glClearColor(0.70f, 0.70f, 0.70f, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < sizeof(SCENES); i++) {
        uint8_t c = SCENES[i];
        if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9'))) {
            LOGF("%c", (char)c);
        } else {
            LOGF("%02x", SCENES[i]);
        }
    }

    struct text_tex img;
    if (!render_text(&img)) {
        LOG("Error render text");
        return 1;
    }

    GLuint img_texid;
    glGenTextures(1, &img_texid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_texid);
    for (int level = 0; img.width > 0 && img.height > 0; level++) {
        glTexImage2D(GL_TEXTURE_2D, level, GL_RED, (GLsizei)img.width, (GLsizei)img.height, 0, GL_RED, GL_UNSIGNED_BYTE, img.data);
        size_t w2 = img.width / 2;
        size_t h2 = img.height / 2;
        for (size_t y = 0; y < h2; y++) {
            for (size_t x = 0; x < w2; x++) {
                img.data[y * w2 + x] = (uint8_t)(
                    0.25 * (
                    (double)img.data[2 * y * img.width + 2 * x] +
                    (double)img.data[2 * y * img.width + 2 * x + 1] +
                    (double)img.data[(2 * y + 1) * img.width + 2 * x] +
                    (double)img.data[(2 * y + 1) * img.width + 2 * x + 1]
                    ));
            }
        }
        img.width = w2;
        img.height = h2;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    LOG("");
    struct scene *scenes;
    size_t nscenes;
    if (!split_scenes(SCENES, sizeof(SCENES), &scenes, &nscenes)) {
        LOG("error splitting scenes");
        return 1;
    }
    // make room for text
    {
        struct scene *tmpscenes = scenes;
        scenes = nullptr;
        scenes = (struct scene *)malloc(sizeof(struct scene) * (nscenes + 1));
        if (!scenes) {
            LOG("Error malloc");
            return 1;
        }
        memcpy(scenes, tmpscenes, sizeof(struct scene) * nscenes);
        free(tmpscenes);
    }
    scenes[nscenes].camera_translation = mkv3(0, 0, 0);
    scenes[nscenes].duration = 6000;
    scenes[nscenes].data = nullptr;
    scenes[nscenes].datasz = 0;
    struct program *progs;
    // +1 for the starting text scene
    progs = (struct program *)malloc((nscenes + 1) * sizeof(struct program));
    GLint *framebuffer_texture_samplers = (GLint *)malloc(MOTIONBLUR_FACTOR * sizeof(GLint));
    for (GLint i = 0; i < (GLint)MOTIONBLUR_FACTOR; i++) {
        framebuffer_texture_samplers[i] = i + 1;
    }
    for (size_t i = 0; i < nscenes + 1; i++) {
        if (i < nscenes) {
            progs[i].id = create_program_from_scene(scenes[i].data, scenes[i].datasz);
        } else {
            progs[nscenes].id = create_text_program();
        }
        progs[i].attr_p = glGetAttribLocation(progs[i].id, "p");
        progs[i].ufrm_width = glGetUniformLocation(progs[i].id, "width");
        progs[i].ufrm_height = glGetUniformLocation(progs[i].id, "height");
        progs[i].ufrm_millis = glGetUniformLocation(progs[i].id, "millis");
        progs[i].ufrm_camera = glGetUniformLocation(progs[i].id, "camera");
        progs[i].ufrm_currentFramebuffer = glGetUniformLocation(progs[i].id, "currentFramebuffer");
        GLint samplers = glGetUniformLocation(progs[i].id, "framessampler");
        glUseProgram(progs[i].id);
        if (samplers >= 0) {
            glUniform1iv(samplers, MOTIONBLUR_FACTOR, framebuffer_texture_samplers);
        }
        glBindFragDataLocation(progs[i].id, 0, "colorBackLeft");
        glBindFragDataLocation(progs[i].id, 1, "colorObject");
        if (i == nscenes) {
            GLuint textsampler_location = glGetUniformLocation(progs[i].id, "textsampler");
            glUniform1i(textsampler_location, 0);
        }
    }
    free(framebuffer_texture_samplers);

    LOGF("total scenes size: %d", (int)sizeof(SCENES));

    // colour attachments and framebuffers
    GLuint third_buffer;
    glGenRenderbuffers(1, &third_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, third_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
    GLuint *framebuffer_color_attachment = (GLuint *)malloc(MOTIONBLUR_FACTOR * 2 * sizeof(GLuint));
    if (!framebuffer_color_attachment) {
        LOG("error malloc");
        return 1;
    }
    GLuint *framebuffer_id = framebuffer_color_attachment + MOTIONBLUR_FACTOR;
    glGenTextures(MOTIONBLUR_FACTOR, framebuffer_color_attachment);
    glGenFramebuffers(MOTIONBLUR_FACTOR, framebuffer_id);
    for (size_t i = 0; i < MOTIONBLUR_FACTOR; i++) {
        glActiveTexture(GL_TEXTURE0 + (GLenum)i + 1);
        glBindTexture(GL_TEXTURE_2D, framebuffer_color_attachment[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id[i]);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, third_buffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                framebuffer_color_attachment[i], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    GLuint vao, buf;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RECTANGLE), RECTANGLE, GL_STREAM_DRAW);

    for (size_t i = 0; i < nscenes; i++) {
        switch_scene(&progs[i], width, height);
    }

    size_t scene = nscenes;
    switch_scene(&progs[scene], width, height);

    /*
    GLuint frontcopyRenderbuffer;
    glGenRenderbuffers(1, &frontcopyRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, frontcopyRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
    GLuint frontcopyFramebuffer;
    glGenFramebuffers(1, &frontcopyFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frontcopyFramebuffer);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, frontcopyRenderbuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glReadBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    */
    // to keep precise 60fps
    // every third frame will be 16ms
    // instead of 17.
    int waiterror = 0;
    Uint32 ticks_start = SDL_GetTicks();
    // bleh. offset with first scene to put back camera on track
    const Uint32 ticks_first = ticks_start + scenes[nscenes].duration;
    Uint32 scene_start = ticks_start;
    unsigned nframes = 0;
    float *rects = (float *)malloc(8 * height * sizeof(float));
    if (!rects) {
        LOG("error malloc");
        return 1;
    }
    size_t framebuffer = 0;
    current_scene.store((int)scene, std::memory_order_release);
    SDL_PauseAudio(0);
    uint8_t *framebuffer_data = nullptr;
    if (movie) {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        framebuffer_data = (uint8_t *)malloc(3 * width * height);
    }
    for (;;) {
        if (ticks_start - scene_start >= scenes[scene].duration) {
            scene_start = scene_start + scenes[scene].duration;
            scene = scene + 1;
            if (scene >= nscenes) {
                scene = 0;
            }
            switch_scene(&progs[scene], width, height);
            current_scene.store((int)scene, std::memory_order_release);
        }
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    width = event.window.data1;
                    height = event.window.data2;
                    glUniform1i(progs[scene].ufrm_width, width);
                    glUniform1i(progs[scene].ufrm_height, height);
                    glViewport(0, 0, width, height);
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
                    for (size_t i = 0; i < MOTIONBLUR_FACTOR; i++) {
                        glActiveTexture(GL_TEXTURE0 + (GLenum)i + 1);
                        glBindTexture(GL_TEXTURE_2D, framebuffer_color_attachment[i]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
                        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id[i]);
                        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_RENDERBUFFER, third_buffer);
                        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                                framebuffer_color_attachment[i], 0);
                        glDrawBuffer(GL_COLOR_ATTACHMENT1);
                        glClearColor(0, 0, 0, 0);
                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                }
            }
        }
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id[framebuffer]);
        glClearColor(0.70f, 0.70f, 0.70f, 1);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        GLuint drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawbuffers);
        if (progs[scene].ufrm_millis >= 0) {
            glUniform1i(progs[scene].ufrm_millis, ticks_start - scene_start);
        }
        if (progs[scene].ufrm_currentFramebuffer >= 0) {
            glUniform1i(progs[scene].ufrm_currentFramebuffer, (GLint)framebuffer);
        }
        if (scene < nscenes) {
            camera = mkcamera(ticks_start - ticks_first, mktranslationm4(scenes[scene].camera_translation));
            glUniformMatrix4fv(progs[scene].ufrm_camera, 1, 0, &camera.c[0].v[0]);
        }
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        //glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_id[framebuffer]);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlendFunc(GL_ONE, GL_ZERO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        if (movie) {
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer_data);
            size_t remaining = width * height * 3;
            while (remaining) {
                remaining = remaining - fwrite(framebuffer_data + width * height * 3 - remaining,
                        1, remaining, fvideoout);
            }
        }
        SDL_GL_SwapWindow(window);
        framebuffer = (framebuffer + 1) % MOTIONBLUR_FACTOR;
        Uint32 allotted = 17;
        if (waiterror == 2) {
            allotted--;
        }
        if (!movie) {
            Uint32 ticks_end = SDL_GetTicks();
            Uint32 frametime = ticks_end - ticks_start;
            if (frametime < allotted) {
                SDL_Delay(allotted - frametime);
                ticks_start += allotted;
            } else {
                ticks_start = SDL_GetTicks();
            }
            waiterror = (waiterror + 1) % 3;
        } else {
            waiterror = (waiterror + 1) % 3;
            ticks_start += allotted;
        }
        nframes++;
    }
}

//AUDIO
void audio_callback(void *userdata, Uint8 *stream, int len) {
    audio_state *as = (audio_state *)userdata;
    Sint16 *buf = (Sint16 *)stream;
    size_t bufsz = len / 2;
    as->current_scene = current_scene.load(std::memory_order_acquire);
    for (size_t i = 0; i < bufsz; i++) {
        buf[i] = audio_gen(as);
        audio_state_advance(as, 1);
    }
    if (movie) {
        size_t remaining = (size_t) len;
        while (remaining) {
            remaining -= fwrite(stream + len - remaining, 1, remaining, faudioout);
        }
    }
}

