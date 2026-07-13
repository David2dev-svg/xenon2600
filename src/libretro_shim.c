#include "libretro_shim.h"
#include "xenon_video.h"
#include "xenon_audio.h"
#include "xenon_input.h"

/* Callback de "environment": o core usa isso pra perguntar coisas ao
   frontend (formato de pixel, opções, etc). No começo, só precisamos
   responder ao essencial pra ele aceitar rodar. Vá adicionando `case`s
   conforme o core reclamar em runtime (log via printf ajuda a ver
   quais comandos ele está pedindo). */
bool shim_environment_cb(unsigned cmd, void *data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
            *(bool *)data = true;
            return true;

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
            return xenon_video_set_pixel_format(*(int *)data);

        default:
            return false; /* "não suportado" — na maioria dos casos tudo bem */
    }
}

void shim_video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (data) {
        xenon_video_blit(data, width, height, pitch);
    }
}

size_t shim_audio_sample_batch_cb(const int16_t *data, size_t frames) {
    xenon_audio_push(data, frames);
    return frames;
}

void shim_input_poll_cb(void) {
    /* O polling em si acontece no loop principal (xenon_input_poll_pads),
       porque libxenon tipicamente exige polling explícito via
       usb_do_poll(). Aqui só existe pra satisfazer a API do core. */
}

int16_t shim_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id) {
    (void)device;
    (void)index;
    return xenon_input_get_button(port, id);
}
