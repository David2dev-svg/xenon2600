#ifndef LIBRETRO_SHIM_H
#define LIBRETRO_SHIM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Copie libretro.h do repositório stella2014-libretro (fica na raiz
   do repo) para core/stella2014-libretro/libretro.h antes de compilar. */
#include "libretro.h"

bool    shim_environment_cb(unsigned cmd, void *data);
void    shim_video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch);
size_t  shim_audio_sample_batch_cb(const int16_t *data, size_t frames);
void    shim_input_poll_cb(void);
int16_t shim_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id);

#endif /* LIBRETRO_SHIM_H */
