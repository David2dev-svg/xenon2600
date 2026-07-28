#include "xenon_audio.h"
#include <string.h>
#include <xenon_sound/sound.h>
#include <xenon_soc/xenon_power.h>

/* Igual ao padrao do mupen64-360 (xenos_audio/audio.c): o processamento
   de audio roda numa thread secundaria dedicada, com stack propria,
   em vez de competir pela stack principal (que no app.lds compartilha
   regiao com o heap, sem tamanho fixo reservado -- um bom motivo pra
   crashar quando adicionamos qualquer coisa nova la). */
#define AUDIO_THREAD_ID 2
static unsigned char g_audio_stack[0x100000]; /* 1MB, igual ao mupen64-360 */

/* O core entrega audio a 31400Hz fixo, o hardware exige 48000Hz fixo.
   48000/31400 reduzido = 240/157 exatamente. */
#define RATIO_NUM      240u
#define RATIO_DEN      157u
#define MAX_RAW_FRAMES 2048
#define MAX_OUT_FRAMES 4096

/* Handoff entre a thread principal (produtor) e a thread de audio
   (consumidor) -- buffer de 1 slot com espera ocupada, igual ao
   thread_enqueue()/thread_bufsize do mupen64-360. */
static volatile int16_t g_raw_buf[MAX_RAW_FRAMES * 2];
static volatile size_t  g_raw_frames = 0;

static int16_t g_out_buf[MAX_OUT_FRAMES * 2];
static unsigned int g_acc = 0;

static inline int16_t swap16(int16_t v) {
    uint16_t u = (uint16_t)v;
    return (int16_t)((u >> 8) | (u << 8));
}

static void xenon_audio_thread_loop(void) {
    for (;;) {
        if (g_raw_frames == 0) continue;

        size_t frames = g_raw_frames;
        size_t out_count = 0;

        for (size_t i = 0; i < frames && out_count < MAX_OUT_FRAMES; i++) {
            int16_t l = g_raw_buf[i * 2 + 0];
            int16_t r = g_raw_buf[i * 2 + 1];

            g_acc += RATIO_NUM;
            while (g_acc >= RATIO_DEN && out_count < MAX_OUT_FRAMES) {
                /* sound.h documenta PCM little-endian; PowerPC e
                   big-endian, entao converte aqui antes de submeter. */
                g_out_buf[out_count * 2 + 0] = swap16(l);
                g_out_buf[out_count * 2 + 1] = swap16(r);
                out_count++;
                g_acc -= RATIO_DEN;
            }
        }

        g_raw_frames = 0; /* libera o produtor */

        if (out_count > 0) {
            int len = (int)(out_count * 2 * sizeof(int16_t));
            int free_space = xenon_sound_get_free();
            if (len > free_space) {
                len = free_space - (free_space % 4);
            }
            if (len > 0) {
                xenon_sound_submit(g_out_buf, len);
            }
        }
    }
}

void xenon_audio_init(void) {
    xenon_sound_init();
    g_acc = 0;
    g_raw_frames = 0;

    xenon_thread_startup();
    xenon_run_thread_task(AUDIO_THREAD_ID,
                           g_audio_stack + sizeof(g_audio_stack) - 0x1000,
                           xenon_audio_thread_loop);
}

void xenon_audio_push(const int16_t *samples, size_t frames) {
    if (!samples || frames == 0) return;
    if (frames > MAX_RAW_FRAMES) frames = MAX_RAW_FRAMES;

    /* Espera a thread de audio esvaziar o slot anterior. */
    while (g_raw_frames) { }

    memcpy((void *)g_raw_buf, samples, frames * 2 * sizeof(int16_t));
    g_raw_frames = frames;
}
