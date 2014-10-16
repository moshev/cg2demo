uniform sampler2D textsampler;

void main() {
    vec2 t = (p + vec3(1.0, 1.0, 0.0)).xy / 2;
    float d = texture(textsampler, t);
    vec4 result = vec4(d, d, d, 1.0);
    color = pow(result / 1.0, vec4(g, g, g, 1.0));
}

