# Makefile — Xenon2600
# Port do core libretro "stella2014-libretro" (Atari 2600) pro Xbox 360
# via libxenon.
#
# ============================================================
# AJUSTE ESTA SEÇÃO com base na saída de ./check-toolchain.sh
# e reaproveitando o que já funciona no seu Makefile do FNF demake.
# ============================================================
DEVKITXENON  ?= $(HOME)/xenon/devkitxenon
CROSS        ?= powerpc-elf-
CORE_ROOT    ?= core/stella2014-libretro
CORE_DIR     := $(CORE_ROOT)/stella
LIBRETRO_DIR := $(CORE_ROOT)

EXTRA_CFLAGS  ?=
EXTRA_LDFLAGS ?=

XENON_MACHDEP := -DXENON -m32 -maltivec -fno-pic -mpowerpc64 -mhard-float
LDSCRIPT      := $(DEVKITXENON)/app.lds
# ============================================================

CC      := $(CROSS)gcc
CXX     := $(CROSS)g++
OBJCOPY := $(CROSS)objcopy
CROSS_STRIP := $(CROSS)strip

BUILD_DIR := build
TARGET    := xenon2600

# --- fontes do nosso shim (frontend libxenon) ---
SHIM_SRC := $(wildcard src/*.c)

# --- fontes oficiais do core, direto do Makefile.common do próprio repo ---
# Isso define SOURCES_CXX, SOURCES_C e INCFLAGS exatamente como o build
# oficial do libretro usa — sem cheat/, gui/, test/ etc. que existem no
# repo mas não fazem parte do core de verdade.
-include $(CORE_ROOT)/Makefile.common

INCLUDES := -Isrc -I$(CORE_ROOT) -I$(DEVKITXENON)/usr/include $(INCFLAGS)
DEFS     := -D__LIBXENON__

CFLAGS   := $(INCLUDES) $(DEFS) $(XENON_MACHDEP) -O2 -Wall $(EXTRA_CFLAGS)
CXXFLAGS := $(CFLAGS) -fno-rtti -std=gnu++98

LDFLAGS := $(XENON_MACHDEP) -L$(DEVKITXENON)/xenon/lib/32 -L$(DEVKITXENON)/usr/lib -n -T $(LDSCRIPT) -lfat -lSDL -lxenon -lm $(EXTRA_LDFLAGS)

ALL_C_SRC   := $(SHIM_SRC) $(SOURCES_C)
ALL_CXX_SRC := $(SOURCES_CXX)

OBJS := $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(ALL_C_SRC)) \
        $(patsubst %.cxx,$(BUILD_DIR)/obj/%.o,$(ALL_CXX_SRC))

.PHONY: all clean toolchain-check core-check help

all: toolchain-check core-check $(BUILD_DIR)/$(TARGET).elf32

help:
	@echo "make toolchain-check  - só verifica se o compilador existe"
	@echo "make core-check       - só verifica se o core foi clonado em $(CORE_ROOT)"
	@echo "make                  - build completo"
	@echo "make clean            - limpa build/"

toolchain-check:
	@command -v $(CC) >/dev/null 2>&1 || { \
	  echo "!! '$(CC)' não encontrado no PATH."; \
	  echo "!! Rode ./check-toolchain.sh para localizar o compilador certo"; \
	  echo "!! e passe CROSS=/caminho/prefixo- e DEVKITXENON=/caminho/sdk"; \
	  echo "!! na linha de comando ou editando o topo deste Makefile."; \
	  exit 1; }
	@echo "Compilador OK: $$(command -v $(CC))"

core-check:
	@test -f "$(CORE_ROOT)/Makefile.common" || { \
	  echo "!! $(CORE_ROOT)/Makefile.common não encontrado."; \
	  echo "!! Clone o core:"; \
	  echo "!!   git clone https://github.com/libretro/stella2014-libretro $(CORE_ROOT)"; \
	  exit 1; }
	@echo "Core OK: $(words $(SOURCES_CXX)) fontes .cxx + $(words $(SOURCES_C)) fontes .c"

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(LDFLAGS) -o $@
	@echo "Link OK -> $@"

$(BUILD_DIR)/$(TARGET).elf32: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O elf32-powerpc --adjust-vma 0x80000000 $< $@
	$(CROSS_STRIP) $@
	@echo "Build final OK -> $@ (copie pra USB/rede pro XeLL carregar)"

$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/%.o: %.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
