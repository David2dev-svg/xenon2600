#!/usr/bin/env python3
import sys

path = sys.argv[1] if len(sys.argv) > 1 else \
    "core/stella2014-libretro/libretro-common/vfs/vfs_implementation.c"

with open(path, "r") as f:
    src = f.read()

replacements = [
    (
        "#  if !defined(VITA)\n#  include <dirent.h>\n#  endif",
        "#  if !defined(VITA) && !defined(__LIBXENON__)\n#  include <dirent.h>\n#  endif",
    ),
    (
        "#  include <sys/stat.h>\n#  include <dirent.h>\n#  include <unistd.h>\n#endif",
        "#  include <sys/stat.h>\n#  if !defined(__LIBXENON__)\n#  include <dirent.h>\n#  endif\n#  include <unistd.h>\n#endif",
    ),
    (
        "#else\n   DIR *directory;\n   const struct dirent *entry;\n#endif",
        "#elif defined(__LIBXENON__)\n   int directory; /* sem suporte real a diretorio neste SDK */\n#else\n   DIR *directory;\n   const struct dirent *entry;\n#endif",
    ),
    (
        "#elif defined(__PSL1GHT__) || defined(__PS3__)\n   return (rdir->error != FS_SUCCEEDED);\n#else\n   return !(rdir->directory);\n#endif",
        "#elif defined(__PSL1GHT__) || defined(__PS3__)\n   return (rdir->error != FS_SUCCEEDED);\n#elif defined(__LIBXENON__)\n   return true; /* diretorios nao suportados: sempre erro */\n#else\n   return !(rdir->directory);\n#endif",
    ),
    (
        "#else\n   rdir->directory       = opendir(name);\n   rdir->entry           = NULL;\n#endif",
        "#elif defined(__LIBXENON__)\n   rdir->directory       = 0; /* forca falha em dirent_check_error acima */\n#else\n   rdir->directory       = opendir(name);\n   rdir->entry           = NULL;\n#endif",
    ),
    (
        "#else\n   return ((rdir->entry = readdir(rdir->directory)) != NULL);\n#endif",
        "#elif defined(__LIBXENON__)\n   return false;\n#else\n   return ((rdir->entry = readdir(rdir->directory)) != NULL);\n#endif",
    ),
    (
        "#else\n   if (!rdir || !rdir->entry)\n      return NULL;\n   return rdir->entry->d_name;\n#endif",
        "#elif defined(__LIBXENON__)\n   return NULL;\n#else\n   if (!rdir || !rdir->entry)\n      return NULL;\n   return rdir->entry->d_name;\n#endif",
    ),
    (
        "#else\n   struct stat buf;\n   char path[PATH_MAX_LENGTH];",
        "#elif defined(__LIBXENON__)\n   return false;\n#else\n   struct stat buf;\n   char path[PATH_MAX_LENGTH];",
    ),
    (
        "#else\n   if (rdir->directory)\n      closedir(rdir->directory);\n#endif",
        "#elif defined(__LIBXENON__)\n   /* nada a fechar */\n#else\n   if (rdir->directory)\n      closedir(rdir->directory);\n#endif",
    ),
]

errors = []
for i, (old, new) in enumerate(replacements, start=1):
    count = src.count(old)
    if count != 1:
        errors.append(f"Patch {i}: encontrado {count}x (esperado 1x)")
    else:
        src = src.replace(old, new)

if errors:
    print("FALHOU — nenhuma alteracao foi salva. Detalhes:\n")
    print("\n".join(errors))
    sys.exit(1)

with open(path, "w") as f:
    f.write(src)

print(f"OK — {len(replacements)} patches aplicados com sucesso em {path}")
