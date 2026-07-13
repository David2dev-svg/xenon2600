#include "xenon_video.h"
#include <stdio.h>
#include <xenos/xenos.h>
#include <SDL/SDL.h>

#define SCREEN_W 1280
#define SCREEN_H 720

static SDL_Surface *g_screen = NULL;

void xenon_video_init(void) {
    xenos_init(VIDEO_MODE_AUTO);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("xenon_video_init: SDL_Init falhou: %s\n", SDL_GetError());
        return;
    }

    g_screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 16,
                                 SDL_SWSURFACE | SDL_FULLSCREEN);
    if (!g_screen) {
        printf("xenon_video_init: SDL_SetVideoMode falhou: %s\n", SDL_GetError());
        return;
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_FillRect(g_screen, NULL, SDL_MapRGB(g_screen->format, 0, 64, 0));
    SDL_Flip(g_screen);
}

void xenon_video_configure(unsigned width, unsigned height) {
    printf("xenon_video_configure: nucleo pediu %ux%u\n", width, height);

    /* Unica mudanca desta rodada: limpa a tela toda AQUI (uma vez),
       em vez de dentro do blit (60x/segundo). Tudo mais do blit
       fica identico a versao que ja funcionou. */
    if (g_screen) {
        SDL_FillRect(g_screen, NULL, 0);
        SDL_Flip(g_screen);
    }
}

bool xenon_video_set_pixel_format(int retro_pixel_format) {
    (void)retro_pixel_format;
    return true;
}

void xenon_video_blit(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!g_screen || !data) return;

    SDL_Surface *src = SDL_CreateRGBSurfaceFrom(
        (void *)data, (int)width, (int)height, 16, (int)pitch,
        0xF800, 0x07E0, 0x001F, 0x0000);
    if (!src) return;

    SDL_Rect dst;
    dst.w = (int)width * 3;
    dst.h = (int)height * 3;
    dst.x = (SCREEN_W - dst.w) / 3;
    dst.y = (SCREEN_H - dst.h) / 3;

    SDL_SoftStretch(src, NULL, g_screen, &dst);
    SDL_FreeSurface(src);

    SDL_Flip(g_screen);
}
