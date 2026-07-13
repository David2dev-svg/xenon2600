#ifndef XENON_VIDEO_H
#define XENON_VIDEO_H

#include <stddef.h>
#include <stdbool.h>

void xenon_video_init(void);
void xenon_video_configure(unsigned width, unsigned height);
bool xenon_video_set_pixel_format(int retro_pixel_format);
void xenon_video_blit(const void *data, unsigned width, unsigned height, size_t pitch);

#endif /* XENON_VIDEO_H */
