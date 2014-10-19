uniform sampler2D textsampler;

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
	vec2 t1 = pow(t, vec2(coming, coming));
	vec2 t2 = vec2(1, 1) - pow((vec2(1, 1) - t), vec2(1 - going, 1 - going));
	float d = mix(texture(textsampler, t1).x, texture(textsampler, t2).x, step(0.5, f));
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

