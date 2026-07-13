#include <stdio.h>
#include <stdlib.h>

#include "libretro_shim.h"
#include "xenon_video.h"
#include "xenon_audio.h"
#include "xenon_input.h"
#include "rom_loader.h"

extern void retro_init(void);
extern void retro_deinit(void);
extern void retro_set_environment(retro_environment_t);
extern void retro_set_video_refresh(retro_video_refresh_t);
extern void retro_set_audio_sample_batch(retro_audio_sample_batch_t);
extern void retro_set_input_poll(retro_input_poll_t);
extern void retro_set_input_state(retro_input_state_t);
extern void retro_get_system_av_info(struct retro_system_av_info *info);
extern bool retro_load_game(const struct retro_game_info *game);
extern void retro_run(void);
extern void retro_unload_game(void);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    xenon_video_init();
    xenon_audio_init();
    xenon_input_init();

    retro_set_environment(shim_environment_cb);
    retro_set_video_refresh(shim_video_refresh_cb);
    retro_set_audio_sample_batch(shim_audio_sample_batch_cb);
    retro_set_input_poll(shim_input_poll_cb);
    retro_set_input_state(shim_input_state_cb);

    retro_init();

    char rom_path[256];
    if (!rom_loader_find_first(rom_path, sizeof(rom_path))) {
        printf("Nenhuma ROM encontrada.\n");
        return 1;
    }

    unsigned char *rom_data = NULL;
    size_t rom_size = 0;
    if (!rom_loader_load(rom_path, &rom_data, &rom_size)) {
        printf("Falha ao carregar ROM: %s\n", rom_path);
        return 1;
    }

    struct retro_game_info game;
    game.path = rom_path;
    game.data = rom_data;
    game.size = rom_size;
    game.meta = NULL;

    if (!retro_load_game(&game)) {
        printf("O core recusou a ROM (%s).\n", rom_path);
        free(rom_data);
        return 1;
    }

    struct retro_system_av_info av_info;
    retro_get_system_av_info(&av_info);
    xenon_video_configure(av_info.geometry.base_width,
                           av_info.geometry.base_height);

    for (;;) {
        xenon_input_poll_pads();
        retro_run();
    }

    retro_unload_game();
    retro_deinit();
    free(rom_data);
    return 0;
}
