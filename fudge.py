#!/usr/bin/env python3
# coding: utf-8

# In[1]:

import sdl2
import sdl2.ext
import sys
import util
import numpy
import ctypes
import os
import os.path
from sdl2 import *
if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0):
    print("WAAH!")
    sys.exit(1)

DIR = os.path.dirname(sys.argv[0])

VS_SRC = open(os.path.join(DIR, 'vertex.glsl'), 'rb').read()

FS_SRC = open(os.path.join(DIR, 'fragment.glsl'), 'rb').read()


WIDTH = 1024
HEIGHT = 768

# In[2]:


class FPSCounter(object):
    def __init__(self):
        self.counts = numpy.ndarray((15,), dtype=numpy.float)
        self.i = 0

    def push(self, time):
        i = self.i
        self.counts[i] = time
        i = (i + 1) % len(self.counts)
        self.i = i
        if (i == 0):
            # end has spaces in to erase previous line
            print('FPS:', self.get(), end='              \r')

    def get(self):
        return (len(self.counts)) / numpy.sum(self.counts)


def render_thread(window):
    import time
    import OpenGL
    import OpenGL.GL
    from OpenGL import GL
    print('ready1')
    FRAME_TIME = 1.0 / 60.0
    C = SDL_GL_CreateContext(window)
    SDL_GL_MakeCurrent(window, C)
    GL.glViewport(0, 0, WIDTH, HEIGHT)
#    GL.glClearColor(0x8A / 255.0, 0xFF / 255.0, 0xC1 / 255.0, 1)
    GL.glClearColor(0.85, 0.85, 0.85, 1.0);
    GL.glEnable(GL.GL_BLEND)
    # blending with premultiplied alpha from shader
    GL.glBlendFunc(GL.GL_ONE, GL.GL_ONE_MINUS_SRC_ALPHA)
    print('ready2')
    PROG = util.GLProgram(VS_SRC, FS_SRC)
    attrloc = GL.glGetAttribLocation(PROG.id, b'p')
    vao = GL.glGenVertexArrays(1)
    GL.glBindVertexArray(vao)
    QUAD = util.GLBuffer(12)
    with QUAD.bound:
        QUAD[:] = numpy.array([-1, -1,
                               1, -1,
                               1, 1,
                               -1, 1],
                              dtype=numpy.float32)
        GL.glEnableVertexAttribArray(attrloc)
        GL.glVertexAttribPointer(attrloc, 2, GL.GL_FLOAT, GL.GL_FALSE, 0, None)
    print('ready3')
    fps = FPSCounter()
    GL.glUseProgram(PROG.id)
    PROG['width'] = float(WIDTH)
    PROG['height'] = float(HEIGHT)
    while True:
        evt = None
        sdlevts = sdl2.ext.get_events()
        for sdlevt in sdlevts:
            if sdlevt.type == SDL_QUIT:
                evt = 'escape'
            if sdlevt.type == SDL_WINDOWEVENT_RESIZED:
                GL.glViewport(0, 0, sdlevt.data1, sdlevt.data2)
                PROG['width'] = float(sdlevt.data1)
                PROG['height'] = float(sdlevt.data2)
        if evt == 'escape':
            SDL_GL_DeleteContext(C)
            SDL_DestroyWindow(window)
            break
        t_start = time.monotonic()
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)
        # PROG['millis'] = 123100
        PROG['millis'] = SDL_GetTicks()
        GL.glDrawArrays(GL.GL_TRIANGLE_FAN, 0, 4)
        SDL_GL_SwapWindow(window)
        t_end = time.monotonic()
        t_sleep = FRAME_TIME - (t_end - t_start)
        if t_sleep > 0:
            time.sleep(t_sleep)
        fps.push(time.monotonic() - t_start)
    print()


# In[4]:

for channel in (SDL_GL_RED_SIZE, SDL_GL_GREEN_SIZE, SDL_GL_BLUE_SIZE):
    SDL_GL_SetAttribute(channel, 8)
SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1)
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)

window = SDL_CreateWindow(b"Super Special Awesome Demo 2014",
                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                          1024, 768,
                          SDL_WINDOW_OPENGL |
                          SDL_WINDOW_ALLOW_HIGHDPI |
                          SDL_WINDOW_RESIZABLE)

render_thread(window)
