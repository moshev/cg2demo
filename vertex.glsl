#version 150

uniform float w;
uniform float h;
in vec2 p;

// pixel center
centroid out vec2 q;

// pixel size... well, half of it
//flat out vec2 pixel;

/*
// 0.0 - 1.0
float timing(int period) {
    return float(millis % period) / float(period - 1);
}

// 0.0 - 1.0 - 0.0
float timing2(int period) {
    float t = timing(period);
    return 2.0 * (0.5 - abs(t - 0.5));
}
*/

void main() {
    pixel = vec2(1.0 / w, 1.0 / h);
    vec2 p1 = p * vec2(max(h / w, 1.0), max(w / h, 1.0));
    vec4 pos = vec4(p1, 0.5, 1.0);
//    pixelcenter = p;
    gl_Position = pos;
}
