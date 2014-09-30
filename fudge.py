
# coding: utf-8

# In[1]:

import sdl2
import sys
import util
import numpy
from sdl2 import *
if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0):
    print("WAAH!")
    sys.exit(1)

VS_SRC = open('shader.vert', 'rb').read()


FS_SRC = open('shader.frag', 'rb').read()

# In[2]:


def render_thread(window):
    import time
    import queue
    import OpenGL
    import OpenGL.GL
    from OpenGL import GL
    print('ready1')
    escape = False
    FRAME_TIME = 1.0 / 60.0
    C = SDL_GL_CreateContext(window)
    SDL_GL_MakeCurrent(window, C)
    GL.glViewport(0, 0, 1024, 768)
    GL.glClearColor(0, 0, 0.5, 1)
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
    sdlevt = SDL_Event()
    while True:
        evt = None
        while SDL_PollEvent(sdlevt) != 0:
            if sdlevt.type == SDL_QUIT:
                evt = 'escape'
        if evt == 'escape':
            SDL_GL_DeleteContext(C)
            SDL_DestroyWindow(window)
            return
        t_start = time.monotonic()
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)
        GL.glUseProgram(PROG.id)
        #PROG['millis'] = 1  # SDL_GetTicks()
        PROG['millis'] = SDL_GetTicks()
        GL.glDrawArrays(GL.GL_TRIANGLE_FAN, 0, 4)
        GL.glUseProgram(0)
        SDL_GL_SwapWindow(window)
        t_end = time.monotonic()
        t_sleep = FRAME_TIME - (t_end - t_start)
        if t_sleep > 0:
            time.sleep(t_sleep)


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
