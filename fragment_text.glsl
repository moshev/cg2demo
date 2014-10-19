uniform sampler2D textsampler;

#define TRANSITION 1

void main() {
    vec2 t = (pixelcenter + vec2(1.0, 1.0)) / 2;
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
    float d;
    if (f < 0.5) {
        d = texture(textsampler, t1);
    } else {
        d = texture(textsampler, t2);
    }
    vec4 result = vec4(d, d, d, 1 - smoothstep(0.8, 1, f));

    vec4 c = result;
    vec2 texcoord = (vec2(1, 1) + screenpixel) * 0.5;
    for (int i = 0; i < MOTIONBLUR_FACTOR; i++) {
        c += texture(framessampler[i], texcoord);
    }
    c /= MOTIONBLUR_FACTOR;
    colorObject = result;
    // gamma correction for standard monitor
    float g = 1.0 / 2.2;
    colorBackLeft = pow(c, vec4(g, g, g, 1.0));
}

