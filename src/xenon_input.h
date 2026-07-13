#ifndef XENON_INPUT_H
#define XENON_INPUT_H

#include <stdint.h>

void    xenon_input_init(void);
void    xenon_input_poll_pads(void);
int16_t xenon_input_get_button(unsigned port, unsigned id);

#endif /* XENON_INPUT_H */
