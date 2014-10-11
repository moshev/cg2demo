#version 150

const float TAU = 6.28318530717958647692;

uniform int width;
uniform int height;
uniform int millis;
uniform mat4 camera;
in vec2 p;

centroid out vec2 pixelcenter;

// pixel size... well, half of it
flat out vec2 pixel;

// 0.0 - 1.0
float timing(int period) {
    return float(millis % period) / float(period - 1);
}

// 0.0 - 1.0 - 0.0
float timing2(int period) {
    float t = timing(period);
    return 2.0 * (0.5 - abs(t - 0.5));
}

void main() {
    pixel = vec2(1.0 / width, 1.0 / height);
    vec2 p1 = p * vec2(max(float(height) / float(width), 1.0), max(float(width) / float(height), 1.0));
    vec4 pos = vec4(p1, 0.5, 1.0);
    pixelcenter = p;
    gl_Position = pos;
}
