
# coding: utf-8

# In[1]:

import sdl2
import sys
import threading
import time
import queue
import util
import numpy
from sdl2 import *
if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0):
    print("WAAH!")
    sys.exit(1);

VS_SRC = br'''
#version 150

in vec2 p;
out vec3 ray;

void main() {
    vec4 pos = vec4(p, 0.5, 1.0);
    ray = vec3(p, -1.0);
    ray = normalize(ray);
    gl_Position = pos;
}
'''


FS_SRC = br'''
#version 150

in vec3 ray;
out vec4 color;

float dist_object(vec3 p) {
    vec3 bmin = vec3(-1.3, -1.3, -3.3);
    vec3 bmax = vec3(-0.3, -0.3, -1.0);
    vec3 dmin = bmin - p;
    vec3 dmax = p - bmax;
    vec3 max1 = max(dmin, dmax);
    vec2 max2 = max(max1.xy, max1.z);
    return max(max2.x, max2.y);
}

float shade(vec3 p) {
    vec3 t1 = cross(ray, vec3(1, 0, 0));
    if (dot(t1, t1) < 0.001) {
        t1 = cross(ray, vec3(0, 1, 0));
    }
    t1 = normalize(t1);
    vec3 t2 = normalize(cross(ray, t1));
    vec3 p1 = p + 0.0001 * t1;
    vec3 p2 = p + 0.0001 * t2;
    p1 = p1 + ray * dist_object(p1);
    p2 = p2 + ray * dist_object(p2);
    return dot(normalize(cross(p2 - p, p1 - p)), vec3(0, -1, 0));
}

void main() {
    vec3 p = vec3(0.0, 0.0, 1.0);
    float d = dist_object(p);
    float epsilon = 1.0e-6;
    for (int i = 0; i < 1000; i++) {
        if (d > epsilon) {
            p += d * ray;
            d = dist_object(p);
        }
    }
    if (abs(d) > epsilon) {
        discard;
    }
    color = vec4(vec3(1.0, 1.0, 1.0) * shade(p), 1.0);
}
'''


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

window = SDL_CreateWindow(b"Super Special Awesome Demo 2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768,
                          SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE)

render_thread(window)