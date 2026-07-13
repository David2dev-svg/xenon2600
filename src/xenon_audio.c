#include "xenon_audio.h"
#include <xenon_sound/sound.h>

void xenon_audio_init(void) {
    xenon_sound_init();
}

void xenon_audio_push(const int16_t *samples, size_t frames) {
    if (!samples || frames == 0) return;

    int len = (int)(frames * 2 * sizeof(int16_t));
    int free_space = xenon_sound_get_free();

    if (len > free_space) {
        len = free_space - (free_space % 4);
        if (len <= 0) return;
    }

    xenon_sound_submit((void *)samples, len);
}
