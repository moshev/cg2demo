#version 150

uniform int width;
uniform int height;
uniform int millis;
uniform int globalMillis;
in vec2 p;

out float millis2;

// pixel center
centroid out vec2 pixelcenter;
centroid out vec2 screenpixel;

// pixel size... well, half of it
//flat out vec2 pixel;

#define SWING_FACTOR 16

void main() {
    //pixel = vec2(1.0 / w, 1.0 / h);
    vec2 whfactor = vec2(max(float(height) / float(width), 1.0), max(float(width) / float(height), 1.0));
    vec2 p1 = p * whfactor;
    vec4 pos = vec4(p1, 0.5, 1.0);
    pixelcenter = p;
    screenpixel = p1;
    millis2 = float(globalMillis) + (p.y + 1) * float(height) * SWING_FACTOR;
    gl_Position = pos;
}
