#version 150

const float TAU = 6.28318530717958647692;

uniform uint width;
uniform uint height;
uniform uint millis;
in vec2 p;

centroid out vec2 pixelcenter;

// pixel size... well, half of it
flat out vec2 pixel;

flat out mat3x3 rotmat;

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
    float angle = -TAU * timing(10000);
    vec3 axis = normalize(vec3(-0.2, 1.0, 0.3));
    rotmat = mat3(vec3(1.0, 0.0, 0.0),
                  vec3(0.0, 1.0, 0.0),
                  vec3(0.0, 0.0, 1.0));
    rotmat = rotmat * cos(angle);
    rotmat = rotmat + sin(angle) * mat3(vec3(0.0, axis.z, -axis.y),
                                        vec3(-axis.z, 0.0, axis.x),
                                        vec3(axis.y, -axis.x, 0.0));
    rotmat = rotmat + (1.0 - cos(angle)) * outerProduct(axis, axis);
    pixel = vec2(1.0 / width, 1.0 / height);
    vec2 p1 = p * vec2(max(height / width, 1.0), max(width / height, 1.0));
    vec4 pos = vec4(p1, 0.5, 1.0);
    pixelcenter = p;
    gl_Position = pos;
}
