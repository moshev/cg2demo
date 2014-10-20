#include <SDL.h>
#include "math3d.h"
#include "audio.h"

std::atomic_int current_scene;

// offsets between notes on a scale
// reverse order (first offset on rightmost bit)
// i.e. 0 1 1 1 0 1 1 (plus 1 on each)
//static const uint8_t scale = 0x3B;

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
uint16_t noteshz_HUNGARIAN[] = {
    262, 294, 311, 370, 392, 415, 494, 523, 587, 622, 734, 784, 831, 988, 1047, 1175
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

static double audio_gen_note_sample_func(int samples, double hz, unsigned func) {
    double t = ((int)(samples * hz) % 44100) / 44099.0;
    double alpha = t * TAU;
    double A = 0.0;//1.0 / pow(2, -12) - 1;//alpha / (1 + pow(2, -12));
    double B = 1.3;//pow(2, -12) - 1;//alpha * (pow(2, -12));
    double v = 0;
    switch (func) {
    case 0:
        // B
        // ⌠
        // ⎮ sin(alpha + x) dx
        // ⌡
        // A
        v = -sin(alpha) * cos(A) + sin(alpha) * cos(B) + cos(alpha) * sin(A) - cos(alpha) * sin(B);
        break;
    case 1:
        v = sin(alpha);
        break;
    case 2:
    {
        // B
        // ⌠
        // ⎮ sin(alpha * x) / exp(|x - 1|) dx
        // ⌡
        // A
        // calculated using sympy
        double a2p1 = alpha * alpha + 1;
        v = - exp(1.0) * alpha * cos(B * alpha) / (a2p1 * exp(  B))
            - exp(1.0) * alpha * sin(B * alpha) / (a2p1 * exp(  B))
            + exp(1.0) *         sin(    alpha) / (a2p1 * exp(1.0))
            + exp(  A) * alpha * cos(A * alpha) / (a2p1 * exp(1.0))
            - exp(  A) *         sin(A * alpha) / (a2p1 * exp(1.0))
            + exp(1.0) *         sin(    alpha) / (a2p1 * exp(1.0));
        v /= 2 - exp(1 - B) - exp(A - 1);
    }
    case 3:
        if (t < 0.5) {
            v = smoothstep(0, 0.5, t);
        } else {
            v = smoothstep(0, 0.5, 1 - t);
        }
        v = v * 2 - 1;
        break;
    default:
        v = 0;
    }
    return v;
}

// main tones
double audio_gen_1(audio_state *as) {
    double attack = 0.05;
    double sustain = 0.1;
    double release = 0.82;
    unsigned overtones = 1;
    double overtone_factor = 2;
    int n = as->note;
    as->note = 0;
    int duration = audio_note_duration(as);
    as->note = 1;
    duration += audio_note_duration(as);
    as->note = 0;
    int s = as->samples;
    if (n % 2) {
        s += audio_note_duration(as);
    }
    as->note = n + 1 - n % 2;
    double hz = audio_note_hz(as);
    double u = (double)s / duration;
    double v = 0;
    double f = 1.0 / 4.0;
    f *= smoothstep(0, attack, u);
    f *= 1 - smoothstep(attack + sustain, attack + sustain + release, u);
    for (unsigned i = 1; i <= overtones; i++) {
        v += audio_gen_note_sample_func(s, hz * i, 2) / pow(overtone_factor, (double)i - 1);
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
    double attack = 0.01;
    double sustain = 0.1;
    double release = 0.8;
    unsigned overtones = 1;
    double overtone_factor = 1.5;
    int s = as->samples;
    int duration = audio_note_duration(as);
    as->note = 0;
    double hz = audio_note_hz(as);
    double u = (double)s / duration;
    double v = 0;
    double f = 1.0 / 5.0;
    f *= smoothstep(0, attack, u);
    f *= 1 - smoothstep(attack + sustain, attack + sustain + release, u);
    for (unsigned i = 1; i <= overtones; i++) {
        v += audio_gen_note_sample_func(s, hz * i, 1) / pow(overtone_factor, (double)i - 1);
    }
    v *= f;
    return v;
}

int16_t audio_gen(const audio_state *as) {
    audio_state asrw = *as;
    double v = 0;
    if (as->current_scene < 16 && as->current_scene > 2) {
        v += audio_gen_1(&asrw);
    }
    asrw = *as;
    v += audio_gen_2(&asrw);
    v = clamp(v, -1, 1);
    return (int16_t)(v * 0x6000);
}
