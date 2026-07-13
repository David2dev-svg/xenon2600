#ifndef XENON_AUDIO_H
#define XENON_AUDIO_H

#include <stdint.h>
#include <stddef.h>

void xenon_audio_init(void);
void xenon_audio_push(const int16_t *samples, size_t frames);

#endif /* XENON_AUDIO_H */
