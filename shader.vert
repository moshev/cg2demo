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
