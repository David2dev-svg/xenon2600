# Xenon2600

Port do core libretro `stella2014-libretro` (Atari 2600) pra Xbox 360,
rodando bare-metal via libxenon.

## Estrutura

```
xenon2600/
├── check-toolchain.sh   # rode isso PRIMEIRO, antes de qualquer build
├── Makefile
├── src/                 # nosso "frontend" — a cola entre libxenon e o core
│   ├── main.c           # entry point: registra callbacks, carrega a ROM, loop principal
│   ├── libretro_shim.c/.h   # implementa as callbacks exigidas pela API libretro
│   ├── xenon_video.c/.h     # TODO: ligar no seu init de vídeo (SDL 1.2) do FNF
│   ├── xenon_audio.c/.h     # TODO: ligar na saída de áudio do libxenon
│   ├── xenon_input.c/.h     # TODO: ligar no usb_init()/usb_do_poll() que você já tem
│   └── rom_loader.c/.h      # TODO: trocar pelo seu carregador de arquivo via USB
└── core/
    └── stella2014-libretro/ # <- clone o repo aqui (instruções abaixo)
```

## Passo 1 — confirmar que o compilador funciona

```bash
bash check-toolchain.sh
```

Isso varre o sistema procurando um `*-gcc` com prefixo powerpc/xenon/ppc,
testa `--version`, `-dumpmachine`, e tenta compilar um `hello.c` mínimo.
No fim ele sugere valores prováveis pra `CROSS` e `DEVKITXENON`.

Se ele não achar nada, o toolchain provavelmente precisa ser
reconstruído — antes de mexer nisso, me manda a saída completa do
script que a gente vê o que falta.

## Passo 2 — buscar o core

```bash
git clone https://github.com/libretro/stella2014-libretro core/stella2014-libretro
```

Depois copie o `libretro.h` da raiz desse repo pra dentro dele mesmo
se ele não estiver lá (algumas versões trazem em `libretro-common/include/`):

```bash
find core/stella2014-libretro -name libretro.h
```

O `src/libretro_shim.h` inclui esse header — o Makefile já aponta
`-Icore/stella2014-libretro` nos includes.

## Passo 3 — ajustar o Makefile

Edite o topo do `Makefile` (ou passe na linha de comando):

```bash
make CROSS=powerpc-elf- DEVKITXENON=$HOME/xenon/devkitxenon
```

Os valores exatos de `CROSS` e `DEVKITXENON` são os que o
`check-toolchain.sh` sugerir. Se você tiver um Makefile funcionando do
FNF demake, copie de lá os `EXTRA_CFLAGS`/`EXTRA_LDFLAGS` (includes do
libxenon, `-T linkscript.lds`, `-lxenon`, etc.) — não inventei esses
valores porque dependem exatamente de como seu SDK está instalado.

```bash
make
```

## O que já está pronto vs. o que falta

**Pronto (esqueleto funcional):**
- Estrutura de build com detecção de erros claros (toolchain ausente, core ausente)
- Todas as callbacks obrigatórias da API libretro (`environment`,
  `video_refresh`, `audio_sample_batch`, `input_poll`, `input_state`)
  já ligadas no `main.c`
- Loop principal chamando `retro_run()`

**Falta você conectar (todos marcados com `TODO` no código):**
- `xenon_video.c` — inicialização real de SDL 1.2 / framebuffer e o
  blit de fato (converter RGB565 → o que sua surface usa)
- `xenon_audio.c` — saída de áudio do libxenon
- `xenon_input.c` — mapear `usb_do_poll()` pros IDs de botão do libretro
- `rom_loader.c` — sua busca real de arquivo `.a26` na USB

Isso é essencialmente o mesmo trabalho de "colar" que você já fez no
port do Lua e no FNF demake — só que agora ligando nos pontos de
entrada que o *core* espera, em vez de escrever a lógica de emulação
você mesmo.

## Dúvidas conhecidas / pontos de atenção

- `stella2014-libretro` é majoritariamente `.cxx`; o Makefile já busca
  esse padrão. Se o repo tiver algum `.cpp` solto, adicione uma regra
  extra no Makefile.
- `RETRO_PIXEL_FORMAT` — confirme em runtime qual formato o core está
  pedindo via `shim_environment_cb` (adicione um `printf` lá dentro
  temporariamente) antes de assumir RGB565.
- Sem `dlopen` nem SO dinâmico: tudo isso é linkado estático num único
  `.elf`, então símbolos duplicados entre core e libxenon (ex: se
  ambos definirem `malloc`/`memcpy` de formas incompatíveis) são o tipo
  de erro mais provável de aparecer primeiro no link.
