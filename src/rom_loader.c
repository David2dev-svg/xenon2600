#include "rom_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <diskio/ata.h>
#include <diskio/disc_io.h>
#include <libfat/fat.h>

/* Igual ao exemplo file/browser: nao existe header publico declarando
   isso, o proprio exemplo oficial faz forward-declare assim. */
int bdev_enum(int handle, const char **name);

/* Nome fixo esperado na raiz do primeiro device de armazenamento
   encontrado (USB, na pratica). Troque aqui se quiser outro nome. */
#define ROM_FILENAME "rom.a26"

bool rom_loader_find_first(char *out_path, size_t out_size) {
    const char *dev_name = NULL;
    int handle;
    char path[64];

    /* Mesma sequencia do file/browser: ata/atapi antes do fat, pra
       cobrir tambem HD/DVD interno alem de USB (usb_init/usb_do_poll
       ja rodaram antes, em xenon_input_init). */
    xenon_ata_init();
    xenon_atapi_init();
    fatInitDefault();

    handle = -1;
    handle = bdev_enum(handle, &dev_name);
    if (handle < 0 || !dev_name) {
        printf("rom_loader: nenhum device de armazenamento encontrado.\n");
        return false;
    }

    snprintf(path, sizeof(path), "%s:/%s", dev_name, ROM_FILENAME);
    printf("rom_loader: tentando %s\n", path);

    if (strlen(path) + 1 > out_size) return false;
    strcpy(out_path, path);
    return true;
}

bool rom_loader_load(const char *path, unsigned char **out_data, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return false; }
    rewind(f);

    unsigned char *buf = (unsigned char *)malloc((size_t)size);
    if (!buf) { fclose(f); return false; }

    if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf);
        fclose(f);
        return false;
    }
    fclose(f);

    *out_data = buf;
    *out_size = (size_t)size;
    return true;
}
