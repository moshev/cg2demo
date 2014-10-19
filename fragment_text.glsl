uniform sampler2D textsampler;

#define TRANSITION 0

void main() {
    vec2 t = (pixelcenter + vec2(1.0, 1.0)) / 2;
//    float f = smoothstep(0, 0.5, timing(8000));
//    float e = mix(0.15, 0.0, pow(f, 2));
//    float d = textureGrad(textsampler, t, vec2(e, e), vec2(e, e)).x;
    //float d = textureLod(textsampler, t, e).x;
    //d = smoothstep(0.01, 0.02, d);
    //d = smoothstep(0.2 * f, 1 - 0.7999 * f, d);
/*
    vec2 dx = vec2(1 - f, 0);
    vec2 dy = vec2(0, 1 - f);
    float d;
    vec2 grad = vec2(e, e);
    d = ( 
      textureGrad(textsampler, clamp(t - dx, 0, 1), grad, grad) + 
      textureGrad(textsampler, clamp(t + dx, 0, 1), grad, grad) + 
      textureGrad(textsampler, clamp(t - dy, 0, 1), grad, grad) +
      textureGrad(textsampler, clamp(t + dy, 0, 1), grad, grad)) * 0.25;
*/
    //t = mod(t, pow(2, (1 - f) * (-2)));
    float f = timing(6000);
    float coming = smoothstep(0, 0.4, f);
    float going = smoothstep(0.6, 1, f);
    vec2 t1;
    vec2 t2;
#if TRANSITION == 0
    t1 = pow(t, vec2(coming, coming));
    t2 = vec2(1, 1) - pow((vec2(1, 1) - t), vec2(1 - going, 1 - going));
#elif TRANSITION == 1
    t1 = clamp(t, 0, coming);
    t2 = clamp(t, going, 1);
#elif TRANSITION == 2
    t1 = clamp(t, 0, pow(coming, 0.5));
    t2 = clamp(t, pow(going, 0.5), 1);
#elif TRANSITION == 3
    t1 = coming * t;
    t2 = (1 - going) * t;
#elif TRANSITION == 4
    t1 = pow(t, vec2(coming, coming));
    t2 = vec2(1, 1) - pow((vec2(1, 1) - t), vec2(1 - going, 1 - going));
    t1 = clamp(t1, pow(1 - coming, 2), 1);
    t2 = clamp(t2, 0, pow(1 - going, 2));
#endif
    float d = mix(texture(textsampler, t1), texture(textsampler, t2), step(0.5, f));
    vec4 result = vec4(d, d, d, 1 - smoothstep(0.8, 1, f));
    float g = 1.0 / 2.2;
    color = pow(result / 1.0, vec4(g, g, g, 1.0));
}

