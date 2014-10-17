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

#include "cg2demo.h"
#include "math3d.h"
#include "scene.h"
#include "scenedsl.h"
#include "shaders.h"
#include "text.h"

#include "protodef.inc"

FILE *flog;

/* read file into memory, return 1 on success, 0 on failure */
int read_file(const char *path, char **text, size_t *sz);

static const bool movie = false;

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

static const uint8_t *taudigits;
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
    if (!get_fragment_shader_text(&fragment_text_glsl, &fragment_text_glslsz)) {
        LOG("error reading fragment shader text-scene source");
        exit(1);
    }


    uint8_t *taudigitsrw = (uint8_t *)malloc(ntaudigits);
    // gen tau
    for (size_t i = 0; i < ntaudigits; i++) {
        taudigitsrw[i] = tau((int)i);
    }
    taudigits = (uint8_t *)&main;
    //taudigits = (uint8_t *)fragment_pre_glsl;
    //taudigits = (uint8_t *)&renderloop;
    //taudigits = SCENES;
    //taudigits = taudigitsrw;
    //taudigits = (uint8_t *)&taudigits;
    SDL_PauseAudio(0);

    SDL_MaximizeWindow(window);
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
        exit(1);
    }

    GLuint img_texid;
    glGenTextures(1, &img_texid);
    glBindTexture(GL_TEXTURE_2D, img_texid);
    for (int level = 0; img.width > 0 && img.height > 0; level++) {
        glTexImage2D(GL_TEXTURE_2D, level, GL_RED, img.width, img.height, 0, GL_RED, GL_UNSIGNED_BYTE, img.data);
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
        exit(1);
    }
    // make room for text
    {
        struct scene *tmpscenes = scenes;
        scenes = nullptr;
        scenes = (struct scene *)malloc(sizeof(struct scene) * (nscenes + 1));
        if (!scenes) {
            LOG("Error malloc");
            exit(1);
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
    for (size_t i = 0; i < nscenes; i++) {
        progs[i].id = create_program_from_scene(scenes[i].data, scenes[i].datasz);
        progs[i].attr_p = glGetAttribLocation(progs[i].id, "p");
        progs[i].ufrm_width = glGetUniformLocation(progs[i].id, "width");
        progs[i].ufrm_height = glGetUniformLocation(progs[i].id, "height");
        progs[i].ufrm_millis = glGetUniformLocation(progs[i].id, "millis");
        progs[i].ufrm_camera = glGetUniformLocation(progs[i].id, "camera");
    }

    // initialise the text scene
    progs[nscenes].id = create_text_program();
    progs[nscenes].attr_p = glGetAttribLocation(progs[nscenes].id, "p");
    progs[nscenes].ufrm_width = glGetUniformLocation(progs[nscenes].id, "width");
    progs[nscenes].ufrm_height = glGetUniformLocation(progs[nscenes].id, "height");
    progs[nscenes].ufrm_millis = glGetUniformLocation(progs[nscenes].id, "millis");
    // reuse like a BOSS
    progs[nscenes].ufrm_camera = glGetUniformLocation(progs[nscenes].id, "textsampler");

    LOGF("total scenes size: %d", (int)sizeof(SCENES));

    GLuint vao, buf;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RECTANGLE), RECTANGLE, GL_STATIC_DRAW);

    for (size_t i = 0; i < nscenes; i++) {
        switch_scene(&progs[i], width, height);
    }

    size_t scene = nscenes;
    switch_scene(&progs[scene], width, height);
    glUniform1i(progs[scene].ufrm_camera, 0);

    // to keep precise 60fps
    // every third frame will be 16ms
    // instead of 17.
    int waiterror = 0;
    Uint32 ticks_start = SDL_GetTicks();
    // bleh. offset with first scene to put back camera on track
    const Uint32 ticks_first = ticks_start + scenes[nscenes].duration;
    Uint32 scene_start = ticks_start;
    for (;;) {
        SDL_Event event;
        if (ticks_start - scene_start >= scenes[scene].duration) {
            scene_start = scene_start + scenes[scene].duration;
            scene = scene + 1;
            if (scene >= nscenes) {
                scene = 0;
            }
            switch_scene(&progs[scene], width, height);
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
                }
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        if (progs[scene].ufrm_millis >= 0) {
            glUniform1i(progs[scene].ufrm_millis, ticks_start - scene_start);
        }
        if (scene < nscenes) {
            camera = mkcamera(ticks_start - ticks_first, mktranslationm4(scenes[scene].camera_translation));
            glUniformMatrix4fv(progs[scene].ufrm_camera, 1, 0, &camera.c[0].v[0]);
        }
        //glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        SDL_GL_SwapWindow(window);
        Uint32 allotted = 16;
        if (waiterror != 2) {
            allotted++;
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
            ticks_start += allotted;
        }
    }
}

// BLUES SCALE
//uint16_t noteshz[] = {
//    262, 311, 349, 370, 392, 466, 523, 622, 699, 740, 784, 932, 1047, 1245, 1397, 1480
//};

// BLUES SCALE with silence
uint16_t noteshz[] = {
    262, 311, 349, 0, 370, 392, 466, 523, 622, 699, //740, 784, 932, 1047, 1245, 1397, 1480
};

//HUNGARIAN MINOR
//uint16_t noteshz[] = {
//    262, 294, 311, 370, 392, 415, 494, 523, 587, 622, 734, 784, 831, 988, 1047, 1175
//};

size_t nnoteshz = sizeof(noteshz) / sizeof(*noteshz);

static inline double clamp(double a, double min, double max) {
    return
        a < min ? min :
        a > max ? max :
                  a;
}

static inline double smoothstep(double min, double max, double a) {
    if (a < min) {
        return 0.0f;
    }
    if (a > max) {
        return 1.0f;
    }
    double t = (a - min) / (max - min);
    t = t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    t = clamp(t, 0.0f, 1.0f);
    return t;
}

static int audio_note_duration(int hz) {
    //return (int)(log1p(hz) * 2000 + 1000);
    return 11025;
}

static void audio_state_advance(audio_state *as, int samples) {
    int s = as->samples + samples;
    int n = as->note;
    int hz = noteshz[taudigits[n] % nnoteshz];
    int duration = audio_note_duration(hz);
    while (s >= duration) {
        s -= duration;
        n = (n + 1) % ntaudigits;
        hz = noteshz[taudigits[n] % nnoteshz];
        duration = audio_note_duration(hz);
    }
    as->samples = s;
    as->note = n;
}

static double audio_gen(const audio_state *as) {
    double attack = 0.1;
    double decay = 0.1;
    int s = as->samples;
    int n = as->note;
    int hz = noteshz[taudigits[n] % nnoteshz];
    int duration = audio_note_duration(hz);
    double u = (double)s / duration;
    double t = ((s * hz) % 44100) / 44099.0;
    double v = 0;
    double alpha = t * TAU;
    double A = -0.5;//1.0 / pow(2, -12) - 1;//alpha / (1 + pow(2, -12));
    double B = 0;//pow(2, -12) - 1;//alpha * (pow(2, -12));
    /*
       for (double factor = 1, displacement = 0; factor < 1.5; factor *= 2.0, displacement += 0.25) {
       v = v + sin((t * factor + displacement) * TAU) / (abs(log(factor)) * 15 + 1);
       }
       */
    //v = -sin(alpha) * cos(A) + sin(alpha) * cos(B) + cos(alpha) * sin(A) - cos(alpha) * sin(B);
    //v = sin(alpha);
    //v = (-exp(B) * alpha + sin(B * alpha) + alpha * cos(B * alpha)) / (exp(B) * (alpha * alpha + 1));
    // integral sin(alpha * (x + 1)) / exp(|x|) dx over A to B
    /*
       v = (exp(-abs(A)) * (sin((A + 1) * alpha) + alpha * cos((A + 1) * alpha))
       - exp(-abs(B)) * (sin((B + 1) * alpha) + alpha * cos((B + 1) * alpha)))
       / (alpha * alpha + 1);
       */
    v = smoothstep(0, 0.5, t) * smoothstep(0, 0.5, 1 - t) * 2;
    v *= smoothstep(0, attack, u);
    v *= smoothstep(0, decay, 1 - u);
    return v;
}

//AUDIO
void audio_callback(void *userdata, Uint8 *stream, int len) {
    audio_state *as = (audio_state *)userdata;
    Sint16 *buf = (Sint16 *)stream;
    size_t bufsz = len / 2;
    double overlap = 0;
    for (size_t i = 0; i < bufsz; i++) {
        double v = 0;
        v = audio_gen(as);
        int hz = noteshz[taudigits[as->note] % nnoteshz];
        int duration = audio_note_duration(hz);
        audio_state as_in = *as;
        if (as->samples + (int)(duration * overlap) >= duration) {
            audio_state_advance(&as_in, (int)(duration * overlap));
            v += audio_gen(&as_in);
            v /= 2;
        }
        buf[i] = (Sint16)(v * 0x4000);
        audio_state_advance(as, 1);
        if (as->samples == 0) {
            *as = as_in;
            audio_state_advance(as, 1);
        }
    }
}
