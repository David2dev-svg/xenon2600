#include "xenon_input.h"
#include <usb/usbmain.h>
#include <input/input.h>

/* Estado dos 4 possiveis controles, atualizado a cada frame por
   xenon_input_poll_pads() e consultado por xenon_input_get_button(). */
static struct controller_data_s g_pad[4];

void xenon_input_init(void) {
    usb_init();
    usb_do_poll();
}

void xenon_input_poll_pads(void) {
    usb_do_poll();
    for (int port = 0; port < 4; port++) {
        get_controller_data(&g_pad[port], port);
    }
}

int16_t xenon_input_get_button(unsigned port, unsigned id) {
    if (port >= 4) return 0;
    struct controller_data_s *p = &g_pad[port];

    /* IDs vem de libretro.h (RETRO_DEVICE_ID_JOYPAD_*). Mapeamento:
       fire principal do 2600 no botao A do controle. */
    switch (id) {
        case 0:  return (int16_t)p->a;               /* JOYPAD_B      -> fire */
        case 1:  return (int16_t)p->x;                /* JOYPAD_Y */
        case 2:  return (int16_t)p->back;             /* JOYPAD_SELECT */
        case 3:  return (int16_t)p->start;            /* JOYPAD_START */
        case 4:  return (int16_t)p->up;               /* JOYPAD_UP */
        case 5:  return (int16_t)p->down;             /* JOYPAD_DOWN */
        case 6:  return (int16_t)p->left;             /* JOYPAD_LEFT */
        case 7:  return (int16_t)p->right;            /* JOYPAD_RIGHT */
        case 8:  return (int16_t)p->b;                /* JOYPAD_A */
        case 9:  return (int16_t)p->y;                /* JOYPAD_X */
        case 10: return (int16_t)p->lb;               /* JOYPAD_L */
        case 11: return (int16_t)p->rb;               /* JOYPAD_R */
        case 12: return (int16_t)(p->lt > 100);       /* JOYPAD_L2 */
        case 13: return (int16_t)(p->rt > 100);       /* JOYPAD_R2 */
        default: return 0;
    }
}
