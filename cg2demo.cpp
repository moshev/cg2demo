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

#include "protodef.inc"

const unsigned int WIDTH = 1024;
const unsigned int HEIGHT = 768;
FILE *flog;

/* rectangle */
GLfloat RECTANGLE[] = {
    -1, -1,
    1, -1,
    1, 1,
    -1, 1
};

/* shader attribute and uniforms */
GLint attr_p;
GLint ufrm_width;
GLint ufrm_height;
GLint ufrm_millis;

int renderloop(SDL_Window *window, SDL_GLContext context);

/* exits with error status and logs error if program wasn't successfully linked */
void check_program(GLuint id);

/* exits with error status and logs error if shader wasn't successfully compiled */
void check_shader(GLuint id);

GLuint create_program_from_files(const char *vspath, const char *fspath);
GLuint create_program(const char *vs, size_t szvs, const char *fs, size_t szfs);

/* read file into memory, return 1 on success, 0 on failure */
int read_file(const char *path, char **text, size_t *sz);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
#if defined(_DEBUG)
    flog = fopen("flog", "w");
    if (!flog) {
        exit(-1);
    }
#else
    flog = nullptr;
#endif
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        LOG("FAILED TO INIT VIDEO\n");
        exit(1);
    }
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
        LOGF("Could not create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

#include "protoget.inc"

    // main loop
    int result = renderloop(window, context);
    SDL_DestroyWindow(window);
    return result;
}

int read_file(const char *path, char **text, size_t *sz) {
    FILE *f = fopen(path, "r");
    if (!f) {
        LOGF("Couldn't read %s: %s", path, strerror(errno));
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0) {
        LOG("error seeking\n");
        fclose(f);
        return 0;
    }
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size);
    if (!buf) {
        LOG("error allocating memory\n");
        fclose(f);
        return 0;
    }
    size_t read = 0;
    while ((size_t)size > read) {
        read += fread(buf + read, (size_t)size - read, 1, f);
    }
    fclose(f);
    *text = buf;
    *sz = (size);
    return 1;
}

GLuint create_program_from_files(const char *vspath, const char *fspath) {
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

void check_shader(GLuint shaderid) {
    GLint compiled;
    glGetShaderiv(shaderid, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        LOG("Compilation error\n");
        GLsizei length;
        glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &length);
        if (length) {
            char *infolog = (char *)malloc(length);
            if (!infolog) {
                LOG("error malloc\n");
                exit(1);
            }
            glGetShaderInfoLog(shaderid, length, &length, infolog);
            LOGF("shader compile log:\n%.*s", (int)length, infolog);
            exit(1);
        }
    }
}

void check_program(GLuint progid) {
    GLint compiled;
    glGetProgramiv(progid, GL_LINK_STATUS, &compiled);
    if (!compiled) {
        LOG("Link error\n");
        GLsizei length;
        glGetProgramiv(progid, GL_INFO_LOG_LENGTH, &length);
        if (length) {
            char *infolog = (char *)malloc(length);
            if (!infolog) {
                LOG("error malloc\n");
                exit(1);
            }
            glGetProgramInfoLog(progid, length, &length, infolog);
            LOGF("program link log:\n%.*s", (int)length, infolog);
            exit(1);
        }
    }
}

GLuint create_shader(GLenum shader_type, const char *src, size_t srcsz) {
    if (shader_type != GL_VERTEX_SHADER && shader_type != GL_FRAGMENT_SHADER) {
        LOGF("unknown shader type: %d\n", (int)shader_type);
        exit(1);
    }
    GLuint shaderid = glCreateShader(shader_type);
    GLint isz = (GLint)srcsz;
    glShaderSource(shaderid, 1, &src, &isz);
    glCompileShader(shaderid);
    check_shader(shaderid);
    return shaderid;
}

GLuint create_program(const char *vs, size_t szvs, const char *fs, size_t szfs) {
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
    return prog;
}

int renderloop(SDL_Window *window, SDL_GLContext context) {
    unsigned int width = WIDTH;
    unsigned int height = HEIGHT;

    glViewport(0, 0, width, height);
    glClearColor(0x8A / 255.0f, 0xFF / 255.0f, 0xC1 / 255.0f, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    GLuint prog = create_program_from_files("vertex.glsl", "fragment.glsl");
    glUseProgram(prog);
    glUniform1ui(ufrm_width, width);
    glUniform1ui(ufrm_height, height);

    GLuint vao, buf;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RECTANGLE), RECTANGLE, GL_STATIC_DRAW);
    glEnableVertexAttribArray(attr_p);
    glVertexAttribPointer(attr_p, 8, GL_FLOAT, 0, 0, 0);

    // to keep precise 60fps
    // every third frame will be 17ms
    // instead of 16.
    int waiterror = 0;
    Uint32 ticks_start = SDL_GetTicks();
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1ui(ufrm_millis, ticks_start);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
        }
        else {
            ticks_start = SDL_GetTicks();
        }
        waiterror = (waiterror + 1) % 3;
    }
}