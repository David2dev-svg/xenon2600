#ifndef ROM_LOADER_H
#define ROM_LOADER_H

#include <stdbool.h>
#include <stddef.h>

bool rom_loader_find_first(char *out_path, size_t out_size);
bool rom_loader_load(const char *path, unsigned char **out_data, size_t *out_size);

#endif /* ROM_LOADER_H */
