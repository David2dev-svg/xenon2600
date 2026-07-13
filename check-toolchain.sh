#!/usr/bin/env bash
# check-toolchain.sh — diagnóstico de toolchain PPC/Xenon no Termux
# Uso:  bash check-toolchain.sh
# Não precisa de root nem de root de filesystem; roda tudo com o usuário atual.

echo "================================================================"
echo "1. PATH atual"
echo "================================================================"
echo "$PATH" | tr ':' '\n'
echo

echo "================================================================"
echo "2. Variáveis de ambiente relevantes"
echo "================================================================"
for v in DEVKITXENON DEVKITPPC XENON_TOOLCHAIN LIBXENON XENON_SDK; do
  val="${!v}"
  printf '%-20s = %s\n' "$v" "${val:-<não definida>}"
done
echo

echo "================================================================"
echo "3. Procurando binários de cross-compiler (powerpc/xenon/ppc)"
echo "================================================================"
SEARCH_ROOTS="$HOME /data/data/com.termux/files /opt /usr/local"
CANDIDATES=$(find $SEARCH_ROOTS -maxdepth 10 \
  \( -iname "*powerpc*gcc*" -o -iname "*xenon*gcc*" -o -iname "*ppc*gcc*" \) \
  -type f 2>/dev/null)

if [ -z "$CANDIDATES" ]; then
  echo "Nenhum binário 'gcc' com prefixo powerpc/xenon/ppc encontrado"
  echo "nos diretórios comuns. Buscando em / (pode demorar)..."
  CANDIDATES=$(find / -xdev -maxdepth 8 \
    \( -iname "*powerpc*gcc*" -o -iname "*xenon*gcc*" -o -iname "*ppc*gcc*" \) \
    -type f 2>/dev/null)
fi

if [ -z "$CANDIDATES" ]; then
  echo "==> Nada encontrado no sistema todo."
else
  echo "$CANDIDATES"
fi
echo

echo "================================================================"
echo "4. Testando cada candidato encontrado"
echo "================================================================"
FIRST_CC=""
for bin in $CANDIDATES; do
  echo "--- $bin ---"
  if [ -x "$bin" ]; then
    "$bin" --version 2>&1 | head -n1
    "$bin" -dumpmachine 2>&1
    [ -z "$FIRST_CC" ] && [[ "$bin" != *g++* ]] && FIRST_CC="$bin"
  else
    echo "(arquivo existe mas não está marcado como executável — rode: chmod +x $bin)"
  fi
  echo
done

echo "================================================================"
echo "5. Procurando instalação de devkitxenon / libxenon"
echo "================================================================"
find $SEARCH_ROOTS -maxdepth 8 \( -iname "*devkitxenon*" -o -iname "*libxenon*" \) 2>/dev/null | head -n 40
echo

echo "================================================================"
echo "6. Teste de compilação mínima"
echo "================================================================"
TMPDIR=$(mktemp -d)
cat > "$TMPDIR/hello.c" <<'EOF'
int main(void) { return 0; }
EOF

if [ -n "$FIRST_CC" ]; then
  echo "Tentando compilar hello.c com: $FIRST_CC"
  if "$FIRST_CC" -c "$TMPDIR/hello.c" -o "$TMPDIR/hello.o" 2>"$TMPDIR/err.log"; then
    echo "OK — o compilador está funcional."
    ls -la "$TMPDIR/hello.o"
    command -v file >/dev/null 2>&1 && file "$TMPDIR/hello.o"
  else
    echo "FALHOU ao compilar. Erro:"
    cat "$TMPDIR/err.log"
  fi
else
  echo "Nenhum compilador C válido encontrado para testar."
fi
rm -rf "$TMPDIR"
echo

echo "================================================================"
echo "7. Histórico do shell (grep por xenon/devkit/toolchain)"
echo "================================================================"
for hist in "$HOME/.bash_history" "$HOME/.zsh_history"; do
  if [ -f "$hist" ]; then
    echo "--- $hist ---"
    grep -inE "xenon|devkit|toolchain" "$hist" | tail -n 25
  fi
done
echo

echo "================================================================"
echo "RESUMO"
echo "================================================================"
if [ -n "$FIRST_CC" ]; then
  echo "Compilador candidato : $FIRST_CC"
  echo "Prefixo a usar       : $(basename "$FIRST_CC" | sed 's/gcc$//')"
  echo "Diretório do SDK     : $(dirname "$(dirname "$FIRST_CC")")"
  echo
  echo "Use esses valores nas variáveis CROSS e DEVKITXENON do Makefile."
else
  echo "Não encontrei nenhum compilador PPC/Xenon instalado."
  echo "Você provavelmente precisa reconstruir o toolchain (Free60"
  echo "devkitxenon) do zero. Guarde a saída deste script antes de eu"
  echo "te ajudar com os próximos passos."
fi
