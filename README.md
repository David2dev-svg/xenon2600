# xenon2600

An Atari 2600 emulator running natively on the Xbox 360 — bare-metal, via [libxenon](https://github.com/Free60Project/libxenon), with no dashboard or Linux involved.

The emulation core is [`stella2014-libretro`](https://github.com/libretro/stella2014-libretro) (a lean libretro-packaged build of [Stella](https://stella-emu.github.io/)). This project is the *frontend*: the layer that wires that core up to real Xbox 360 hardware (video, audio, controller input, and ROM loading from USB), with no RetroArch and no OS — a single `.elf32` binary loaded directly by XeLL or Aurora.

> **Status: alpha (`v0.1.0-alpha`)** — functional, with one known audio limitation. See [Known issues](#known-issues--todo).

## What works

- ✅ **Video** — the core's framebuffer (RGB565) scaled 2x via `SDL_SoftStretch`, fullscreen 720p
- ✅ **Controller** — Xbox 360 USB controller mapped to the libretro input API
- ✅ **ROM loading** — reads `rom.a26` from the root of the first detected USB device (via `libfat`)
- ⚠️ **Audio** — plays, but at the wrong speed/pitch (see below)

## Architecture

```
xenon2600/
├── src/                  # frontend: the glue between libxenon and the core
│   ├── main.c            # entry point, main loop
│   ├── libretro_shim.c   # implements the callbacks required by the libretro API
│   ├── xenon_video.c     # SDL 1.2 (xenos + SDL_SoftStretch)
│   ├── xenon_audio.c     # xenon_sound_submit
│   ├── xenon_input.c     # get_controller_data (libxenon input driver)
│   └── rom_loader.c      # fatInitDefault + bdev_enum + fopen
├── core/
│   └── stella2014-libretro/   # separate clone (not vendored, see Build)
└── Makefile
```

## Build — from scratch

Documenting everything that was needed here, since no single piece was ready to go out of the box:

### 1. Toolchain (devkitxenon)

```bash
git clone https://github.com/Free60Project/libxenon ~/libxenon
cd ~/libxenon/toolchain
bash build-xenon-toolchain toolchain
bash build-xenon-toolchain libxenon
bash build-xenon-toolchain filesystems   # fat-xenon (libfat), ext2fs-xenon, xtaflib
```

```bash
export DEVKITXENON=/usr/local/xenon
export PATH=$DEVKITXENON/bin:$PATH
```

### 2. SDL (libSDLXenon)

```bash
git clone https://github.com/lantus/libSDLXenon ~/libxenon/libSDLXenon
cd ~/libxenon/libSDLXenon
```

Needs a one-line patch before it builds — the current libxenon SDK renamed the controller struct's `select` field to `back`:

```bash
sed -i 's/curpad\.select/curpad.back/' src/joystick/xenon/SDL_xenonjoystick.c
make -f Makefile.xenon install
```

### 3. Core (stella2014-libretro)

```bash
git clone https://github.com/libretro/stella2014-libretro core/stella2014-libretro
```

Needs a patch to disable directory support in `libretro-common` — this toolchain's `newlib` never implemented `opendir`/`readdir` (`dirent.h` even has a deliberate `#error` in it). See [`patches/fix_libxenon_vfs.py`](patches/fix_libxenon_vfs.py) — it adds an `#elif defined(__LIBXENON__)` branch at 9 points in `libretro-common/vfs/vfs_implementation.c`, making the directory functions always report "unsupported" (this doesn't affect ROM loading, which goes through our own `rom_loader.c` instead).

```bash
python3 patches/fix_libxenon_vfs.py core/stella2014-libretro/libretro-common/vfs/vfs_implementation.c
```

### 4. Final build

```bash
make DEVKITXENON=/usr/local/xenon CROSS=xenon-
```

Produces `build/xenon2600.elf32`. Copy it to USB/network and load it from XeLL. Put a ROM named `rom.a26` in the root of the same drive.

## Known issues / TODO

- **Audio plays at the wrong speed.** The core outputs audio at a fixed 31400Hz (`stella2014-libretro/libretro.cxx`), but the hardware (`xenon_sound_submit`) requires a fixed 48000Hz. Without resampling, audio plays back ~1.53x too fast/high-pitched. A floating-point resampler attempt caused an `Exception vector (0x700)` — suspected stack overflow (Stella's C++ code already has fairly deep call chains). Next attempt: a fixed-point integer resampler, avoiding `double` entirely.
- No on-screen ROM selection — loads a fixed `rom.a26`.
- No save states.
- `SDL_UpdateRect` (partial screen update) triggers an `Exception vector` on this SDL port — only a full `SDL_Flip` works. Not fully root-caused (suspected: an unimplemented driver hook in this specific SDL port).
- Writing files (`fopen` in `"w"` mode) also triggers an `Exception vector` — write support in this `libfat` build appears untested/broken in this environment. Reading works fine.

## Credits

- [Free60 Project](https://free60.org/) / [libxenon](https://github.com/Free60Project/libxenon) — the bare-metal Xbox 360 toolchain and SDK
- [lantus/libSDLXenon](https://github.com/lantus/libSDLXenon) — the SDL 1.2 port
- [Stella](https://stella-emu.github.io/) and [libretro/stella2014-libretro](https://github.com/libretro/stella2014-libretro) — the emulation core itself
  Built by dWhoami and LBarroso with development assistance from Claude (Anthropic).
## License

The `stella2014-libretro` core is licensed under GPLv2 (inherited from Stella). Since this project includes and links that code, it should also be distributed under GPLv2 — check the exact terms in Stella's `LICENSE` before publishing; this isn't legal advice.
