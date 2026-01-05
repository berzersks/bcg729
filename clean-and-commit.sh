#!/bin/bash
set -e

echo "🧹 Limpando arquivos de build..."
make clean 2>/dev/null || true

echo "🗑️  Removendo arquivos gerados..."
rm -f config.log config.status
rm -rf .libs modules
rm -f *.o *.lo *.la
rm cmak*

echo "📋 Status do git antes da limpeza:"
git status --short

echo ""
echo "✅ Limpeza concluída!"
echo ""
echo "📝 Pronto para commit. Arquivos modificados:"
git status --short
