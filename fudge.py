
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

VS_SRC = br'''
#version 150

in vec2 p;
out vec3 inray;

void main() {
    vec2 p1 = p * vec2(1.0, 1024.0 / 768.0);
    vec4 pos = vec4(p1, 0.5, 1.0);
    inray = vec3(p, -1.0);
    inray = normalize(inray);
    gl_Position = pos;
}
'''


FS_SRC = br'''
#version 150

uniform int millis;
in vec3 inray;
out vec4 color;

vec3 ray;

/*cube*/
float cube(vec3 p) {
    vec3 bmin = vec3(-0.7, -0.7, -0.7);
    vec3 bmax = vec3(-0.2, -0.2, -0.2);
    vec3 dmin = bmin - p;
    vec3 dmax = p - bmax;
    vec3 max1 = max(dmin, dmax);
    vec2 max2 = max(max1.xy, max1.z);
    return max(max2.x, max2.y);
}


/*sphere*/
float sphere(vec3 p, vec3 c, float r) {
    return distance(c, p) - r;
}

float timing(int period) {
    return float(millis % period) / float(period - 1);
}

float dist_object(vec3 p) {
    float t = timing(4000);
    t = 2.0 * abs(t - 0.5);
    float u = (1.0 - t);
    // this piece is nonsensical but without it
    // we get a black screen, fuck you nVidia
    // fuck you with a rusty rake
    if (u < 0.0) {
        u = 0.0;
    }
    vec3 centre = vec3(-0.2, -0.2, -0.4);
    return (u * sphere(p, centre, 0.25) + t * cube(p));
/*
    return cube(p + vec3(sin(3.141259 * fract(p.x + t)),
                         sin(3.141259 * fract(p.y + t)),
                         sin(3.141259 * fract(p.z + t))));
                        */
}


vec4 trace(vec3 p) {
    float d = dist_object(p);
    float epsilon = 4.0e-07;
    for (int i = 0; i < 1024; i++) {
        if (d > epsilon) {
            p += d * ray;
            d = dist_object(p);
        }
    }
    if (d > epsilon) {
        return vec4(d, d, d, 0);
    }
    p += d * ray;
    return vec4(p, 1);
}

vec3 shade(vec3 p, vec3 c) {
    vec3 t1 = cross(ray, vec3(1, 0, 0));
    vec3 light1 = normalize(vec3(-0.5, -0.2, -0.1));
    vec3 light2 = normalize(vec3(0.1, -0.0, -1.0));
    if (dot(t1, t1) < 0.001) {
        t1 = cross(ray, vec3(0, 1, 0));
    }
    t1 = normalize(t1);
    vec3 t2 = normalize(cross(ray, t1));
    vec3 p1 = p + 0.0001 * t1;
    vec3 p2 = p + 0.0001 * t2;
    vec4 q1 = trace(p1 - ray);
    vec4 q2 = trace(p2 - ray);
    if (q1.w > 0) {
        p1 = q1.xyz;
    }
    if (q2.w > 0) {
        p2 = q2.xyz;
    }
    vec3 n = normalize(cross(p1 - p, p2 - p));
    return min((max(dot(n, light1), 0.0) +
           max(dot(n, light2), 0.0)) * 0.5 * c, c);
//    return (vec3(1.0, 1.0, 1.0) + normalize(cross(p1 - p, p2 - p))) * 0.5;
}

void main() {
    vec3 p = vec3(0.0, 0.0, 1.0);
    float angle = 6.28318530717958647692 * timing(20000);
    vec3 axis = normalize(vec3(-0.2, 1.0, 0.3));
    mat3x3 rotmat = mat3(vec3(1.0, 0.0, 0.0),
                         vec3(0.0, 1.0, 0.0),
                         vec3(0.0, 0.0, 1.0));
    rotmat = rotmat * cos(angle);
    rotmat = rotmat + sin(angle) * mat3(vec3(0.0, axis.z, -axis.y),
                                        vec3(-axis.z, 0.0, axis.x),
                                        vec3(axis.y, -axis.x, 0.0));
    rotmat = rotmat + (1.0 - cos(angle)) * outerProduct(axis, axis);
    p = rotmat * p;
    ray = rotmat * inray;
    vec4 q = trace(p);
    if (q.w < 1.0) {
        discard;
    }
    p = q.xyz;
    color = vec4(shade(p, vec3(1.0, 1.0, 1.0)), 1.0);
    color = pow(color, vec4(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0));
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

window = SDL_CreateWindow(b"Super Special Awesome Demo 2",
                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                          1024, 768,
                          SDL_WINDOW_OPENGL |
                          SDL_WINDOW_ALLOW_HIGHDPI |
                          SDL_WINDOW_RESIZABLE)

render_thread(window)
