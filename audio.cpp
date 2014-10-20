#include <SDL.h>
#include "math3d.h"
#include "audio.h"

std::atomic_int current_scene;

// offsets between notes on a scale
// reverse order (first offset on rightmost bit)
// i.e. 0 1 1 1 0 1 1 (plus 1 on each)
//static const uint8_t scale = 0x3B;

int8_t notesoffsets_DOREMI[] = {
    0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26
};

int8_t notesoffsets_BLUES[] = {
    3, 6, 8, 9, 10, 13, 15, 18, 20, 21, 22, 25, 27, 30, 32, 33, 35,
};

int8_t notesoffsets_MIDDLE_BLUES[] = {
    -5, -2, 0, 1, 2, 5, 7, 11, 12, 13, 14, 17, 19, 23, 24, 25, 27,
};

int8_t notesoffsets_DEEP_BLUES[] = {
    -13, -10, -8, -7, -6, -3, -1, 3, 4, 5, 6, 9, 11, 14, 16, 17, 19,
};

//HUNGARIAN MINOR
int8_t notesoffsets_HUNGARIAN[] = {
    3, 5, 6, 9, 10, 11, 15, 16, 18, 19, 22, 23, 24, 25, 26, 27
};

#define NOTESOFFSETS notesoffsets_BLUES

const int8_t *notesoffsets = NOTESOFFSETS;
const size_t nnotesoffsets = sizeof(NOTESOFFSETS) / sizeof(*NOTESOFFSETS);

static int audio_note_duration(const audio_state *as) {
    //return (int)(log1p((double)hz) * 2000 + 1000);
    //return 5555;
    //return 11025;
    //return as->current_scene % 2 ? 5555 : 11025;
    //return 22050;
    return (as->note % 2 ? 8000 : 5450);
    //return 11025;
    //return 6666;
    //return 8000;
}

static int audio_scale_duration(int scale) {
    return scale == 0 ? 32 : 8;
}

static double audio_note_hz(const audio_state *as) {
    int n = as->note;
    int offset = n % 2 ?
        notesoffsets[(as->taudigits[n / 2] % (nnotesoffsets - 1)) + 1] :
        notesoffsets[0];
    double hz = 220.0 * pow(2.0, (offset + as->scale) / 12.0);
    return hz;
}

void audio_state_advance(audio_state *as, int samples) {
    int s = as->samples + samples;
    int n = as->note;
    int t = as->scalenotes;
    int duration = audio_note_duration(as);
    while (s >= duration) {
        s -= duration;
        n = n + 1;
        if (n % 2 && (size_t)(n / 2) >= as->ntaudigits) {
            n = 0;
        }
        as->samples = s;
        as->note = n;
        duration = audio_note_duration(as);
        t++;
    }
    duration = audio_scale_duration(as->scale);
    while (t >= duration) {
        t -= duration;
        switch (as->scale) {
        case 0:
            as->scale = 7;
            break;
        case 7:
            as->scale = 5;
            break;
        default:
            as->scale = 0;
            break;
        }
        duration = audio_scale_duration(as->scale);
    }
    as->scalenotes = t;
    as->samples = s;
    as->note = n;
}

#define AUDIO_GEN_FUNC 1

static double audio_gen_note_sample(int samples, double hz) {
    double t = ((int)(samples * hz) % 44100) / 44099.0;
    double alpha = t * TAU;
    double A = 0.2;//1.0 / pow(2, -12) - 1;//alpha / (1 + pow(2, -12));
    double B = 1.1;//pow(2, -12) - 1;//alpha * (pow(2, -12));
    double v = 0;
#if AUDIO_GEN_FUNC == 0
    // B
    // ⌠
    // ⎮ sin(alpha + x) dx
    // ⌡
    // A
    v = -sin(alpha) * cos(A) + sin(alpha) * cos(B) + cos(alpha) * sin(A) - cos(alpha) * sin(B);
#elif AUDIO_GEN_FUNC == 1
    (void) A;
    (void) B;
    v = sin(alpha);
#elif AUDIO_GEN_FUNC == 2
    // B
    // ⌠
    // ⎮ sin(alpha * x) / exp(|x - 1|) dx
    // ⌡
    // A
    // calculated using sympy
    double a2p1 = alpha * alpha + 1;
    v = - exp(1.0) * alpha * cos(B * alpha) / (a2p1 * exp(B  ))
        - exp(1.0) * alpha * sin(B * alpha) / (a2p1 * exp(B  ))
        + exp(1.0) *         sin(    alpha) / (a2p1 * exp(1.0))
        + exp(A  ) * alpha * cos(A * alpha) / (a2p1 * exp(1.0))
        - exp(A  ) *         sin(A * alpha) / (a2p1 * exp(1.0))
        + exp(1.0) *         sin(    alpha) / (a2p1 * exp(1.0));
    v /= 2 - exp(1 - B) - exp(A - 1);
#elif AUDIO_GEN_FUNC == 3
    (void) A;
    (void) B;
    (void) alpha;
    if (t < 0.5) {
        v = smoothstep(0, 0.5, t);
    } else {
        v = smoothstep(0, 0.5, 1 - t);
    }
    v = v * 2 - 1;
#endif
    return v;
}

// main tones
double audio_gen_1(audio_state *as) {
    double attack = 0.02;
    double sustain = 0.1;
    double release = 0.6;
    unsigned overtones = 1;
    double overtone_factor = 1.41;
    int s = as->samples;
    int duration = audio_note_duration(as);
    as->note -= 1 - as->note % 2;
    double hz = audio_note_hz(as);
    double u = (double)s / duration;
    double v = 0;
    double f = 1.0 / 2.0;
    f *= smoothstep(0, attack, u);
    f *= 1 - smoothstep(attack + sustain, attack + sustain + release, u);
    for (unsigned i = 1; i <= overtones; i++) {
        v += audio_gen_note_sample(s, hz * i) / pow(overtone_factor, (double)i);
    }
    v *= f;
    /*
    audio_state asrw = *as;
    if (as->note % 2 == 0) {
        asrw.scale += 4;
        hz = audio_note_hz(&asrw);
        v += f * audio_gen_note_sample(as->samples, hz);
        asrw.scale += 3;
        hz = audio_note_hz(&asrw);
        v += f * audio_gen_note_sample(as->samples, hz);
    } else {
        v *= 3;
    }
    */
    //v -= 1;
    //v = smoothstep(0, 0.5, 0.5 - abs(t - 0.5)) * 2 - 1;
    return v;
}

// bass line
double audio_gen_2(audio_state *as) {
    as->scale -= 12;
    as->note -= as->note % 2;
    double attack = 0.05;
    double sustain = 0.0;
    double release = 0.9;
    unsigned overtones = 16;
    double overtone_factor = 1.41;
    int s = as->samples;
    double hz = audio_note_hz(as);
    int duration = audio_note_duration(as);
    double u = (double)s / duration;
    double v = 0;
    double f = 1.0 / 6.0;
    f *= smoothstep(0, attack, u);
    f *= 1 - smoothstep(attack + sustain, attack + sustain + release, u);
    for (unsigned i = 1; i <= overtones; i++) {
        v += audio_gen_note_sample(s, hz * i) / pow(overtone_factor, (double)i);
    }
    v *= f;
    return v;
}

// low-frequency dramatic
double audio_gen_3(audio_state *as) {
    double attack = 0.01;
    double sustain = 0.175;
    double release = 0.820;
    double vibrato = 0.5;
    int vibrato_factor = 4;
    unsigned overtones = 1;
    double overtone_factor = 1;
    int s = as->samples;
    int n = as->note;
    for (int i = 0; i < n % 2; i++) {
        as->note = i;
        s += audio_note_duration(as);
    }
    as->note = 1;
    int duration = audio_note_duration(as);
    as->note = 0;
    duration += audio_note_duration(as);
    double hz = audio_note_hz(as);
    double u = (double)s / duration;
    double v = 0;
    double f = 1.0 / 6.0;
    double fhz = smoothstep(0, attack, u);
    fhz *= 1 - smoothstep(attack + sustain, attack + sustain + release, u);
    f *= pow(sin(fhz * TAU * 0.25), 2);
    for (unsigned i = 1; i <= overtones; i++) {
        v += audio_gen_note_sample(s, hz * i) / pow(overtone_factor, (double)i);
    }
    v *= f;
    as->scale = 12 * vibrato_factor;
    hz = audio_note_hz(as);
    v *= (audio_gen_note_sample(s, hz) + 1.0) * 0.5 * vibrato + 1 - vibrato;
    /*
    audio_state asrw = *as;
    if (as->note % 2 == 0) {
    asrw.scale += 4;
    hz = audio_note_hz(&asrw);
    v += f * audio_gen_note_sample(as->samples, hz);
    asrw.scale += 3;
    hz = audio_note_hz(&asrw);
    v += f * audio_gen_note_sample(as->samples, hz);
    } else {
    v *= 3;
    }
    */
    //v -= 1;
    //v = smoothstep(0, 0.5, 0.5 - abs(t - 0.5)) * 2 - 1;
    return v;
}

int16_t audio_gen(const audio_state *as) {
    audio_state asrw = *as;
    double v = 0;
    v += audio_gen_1(&asrw);
    asrw = *as;
    v += audio_gen_2(&asrw);
    asrw = *as;
    v += audio_gen_3(&asrw);
    v = clamp(v, -1, 1);
    return (int16_t)(v * 0x6000);
}
