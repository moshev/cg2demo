#if !defined(CG2_AUDIO_H)
#define CG2_AUDIO_H

#include <stdint.h>
#include <atomic>

extern const int8_t *notesoffsets;
extern const size_t nnotesoffsets;
extern std::atomic_int current_scene;

struct audio_state {
    unsigned samples;
    int note;
    int scale;
    int scalenotes;
    int current_scene;
    const uint8_t *taudigits;
    size_t ntaudigits;
};

// advance by how many samples
// assuming 44100hz
void audio_state_advance(audio_state *as, int samples);

// generate one sample
// assuming 44100hz
int16_t audio_gen(const audio_state *as);

#endif
