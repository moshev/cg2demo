#version 150

uniform float width;
uniform float height;
in vec2 p;

centroid out vec2 pixelcenter;

// pixel size... well, half of it
flat out vec2 pixel;

void main() {
	pixel = vec2(1.0 / width, 1.0 / height);
    vec2 p1 = p * vec2(max(height / width, 1.0), max(width / height, 1.0));
    vec4 pos = vec4(p1, 0.5, 1.0);
	pixelcenter = p;
    gl_Position = pos;
}
