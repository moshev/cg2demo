#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <SDL.h>
#include <SDL_opengl.h>

const int WIDTH = 1024;
const int HEIGHT = 768;

int renderloop(SDL_Window *window, SDL_GLContext context);

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    FILE *flog = fopen("flog", "w");
    if (!flog) {
        exit(-1);
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        fputs("FAILED TO INIT VIDEO\n", flog);
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
        fprintf(flog, "Could not create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // main loop
    int result = renderloop(window, context);
    SDL_DestroyWindow(window);
    return result;
}

int renderloop(SDL_Window *window, SDL_GLContext context) {
    int width = WIDTH;
    int height = HEIGHT;

    glViewport(0, 0, width, height);
    glClearColor(0x8A / 255.0, 0xFF / 255.0, 0xC1 / 255.0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

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

        SDL_GL_SwapWindow(window);
        Uint32 allotted = 16;
        if (waiterror == 2) {
            allotted++;
        }
        Uint32 ticks_end = SDL_GetTicks();
        Uint32 frametime = ticks_end - ticks_start;
        if (frametime < allotted) {
            SDL_Delay(allotted - frametime);
        }
        ticks_start += allotted;
        waiterror = (waiterror + 1) % 3;
    }
}