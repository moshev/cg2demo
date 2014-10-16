uniform sampler2D textsampler;

void main() {
    vec2 t = (pixelcenter + vec2(1.0, 1.0)) / 2;
    float fac = 1.1 - 2 * clamp(timing(8000), 0, 0.5);
    float d = texture(textsampler, t).x;
    d = smoothstep(fac, fac * 2, d);
    vec4 result = vec4(d, d, d, 1.0);
    float g = 1.0 / 2.2;
    color = pow(result / 1.0, vec4(g, g, g, 1.0));
}

