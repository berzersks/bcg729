#!/usr/bin/env bash
set -e

echo "🧹 Limpando artefatos de build (phpize / cmake / autotools)..."

# ---------
# Diretórios lixo
# ---------
DIRS_TO_REMOVE=(
  build
  buildroot
  CMakeFiles
  autom4te.cache
  .deps
  .libs
  tmp
)

for d in "${DIRS_TO_REMOVE[@]}"; do
  [ -d "$d" ] && echo "Removendo diretório: $d" && rm -rf "$d"
done

# ---------
# Arquivos lixo
# ---------
FILES_TO_REMOVE=(
  Makefile
  Makefile.in
  configure
  config.log
  config.status
  libtool
  aclocal.m4
  install-sh
  missing
  depcomp
  compile
  ltmain.sh
  cmake_install.cmake
  CMakeCache.txt
)

for f in "${FILES_TO_REMOVE[@]}"; do
  [ -f "$f" ] && echo "Removendo arquivo: $f" && rm -f "$f"
done

# ---------
# Limpeza profunda por padrão conhecido
# ---------
echo "Removendo artefatos comuns..."

find . -type f \( \
    -name "*.o" \
 -o -name "*.lo" \
 -o -name "*.la" \
 -o -name "*.a" \
 -o -name "*.so" \
 -o -name "*.so.*" \
 -o -name "*.dylib" \
 -o -name "*.dll" \
 -o -name "*.exe" \
 -o -name "*.out" \
 -o -name "*.gcno" \
 -o -name "*.gcda" \
 -o -name "*.gcov" \
 \) -print -delete

# ---------
# NÃO TOCA EM:
# .c .h .md .php .json .yml .yaml LICENSE README*
# ---------

echo "✅ Repositório limpo."
echo "📦 Apenas código-fonte, docs e configs preservados."
