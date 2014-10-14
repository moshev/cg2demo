#version 150

uniform int width;
uniform int height;
in vec2 p;

// pixel center
centroid out vec2 pixelcenter;

// pixel size... well, half of it
//flat out vec2 pixel;

void main() {
    //pixel = vec2(1.0 / w, 1.0 / h);
    vec2 p1 = p * vec2(max(float(height) / float(width), 1.0), max(float(width) / float(height), 1.0));
    vec4 pos = vec4(p1, 0.5, 1.0);
    pixelcenter = p;
    gl_Position = pos;
}
