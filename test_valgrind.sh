#!/bin/bash
# Script para executar testes de vazamento de memória com Valgrind

set -e

echo "========================================================================="
echo "  TESTE DE VAZAMENTO DE MEMÓRIA COM VALGRIND"
echo "========================================================================="
echo

# Verifica se valgrind está instalado
if ! command -v valgrind &> /dev/null; then
    echo "[ERRO] Valgrind não está instalado!"
    echo "Para instalar no Ubuntu/Debian: sudo apt-get install valgrind"
    echo "Para instalar no Fedora/RHEL: sudo dnf install valgrind"
    exit 1
fi

# Verifica se o script PHP existe
if [ ! -f "test_memory_leak.php" ]; then
    echo "[ERRO] Arquivo test_memory_leak.php não encontrado!"
    exit 1
fi

# Cria diretório para logs
mkdir -p valgrind_logs
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

echo "[INFO] Executando teste básico de vazamento..."
echo

# Teste 1: Leak check completo
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file="valgrind_logs/leak_full_${TIMESTAMP}.log" \
    php test_memory_leak.php

echo
echo "[OK] Teste completo. Log salvo em: valgrind_logs/leak_full_${TIMESTAMP}.log"
echo

# Teste 2: Com alocador Zend desabilitado (mais preciso)
echo "[INFO] Executando teste com alocador Zend desabilitado..."
echo

USE_ZEND_ALLOC=0 valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file="valgrind_logs/leak_nozend_${TIMESTAMP}.log" \
    php test_memory_leak.php

echo
echo "[OK] Teste sem Zend completo. Log salvo em: valgrind_logs/leak_nozend_${TIMESTAMP}.log"
echo

# Teste 3: Detecção de erros de memória
echo "[INFO] Executando teste de erros de memória..."
echo

USE_ZEND_ALLOC=0 valgrind \
    --tool=memcheck \
    --leak-check=yes \
    --show-reachable=yes \
    --num-callers=20 \
    --track-fds=yes \
    --track-origins=yes \
    --log-file="valgrind_logs/memcheck_${TIMESTAMP}.log" \
    php test_memory_leak.php

echo
echo "[OK] Teste memcheck completo. Log salvo em: valgrind_logs/memcheck_${TIMESTAMP}.log"
echo

# Análise dos resultados
echo "========================================================================="
echo "  ANÁLISE DOS RESULTADOS"
echo "========================================================================="
echo

echo "Resumo de vazamentos encontrados:"
echo

for log in valgrind_logs/leak_*_${TIMESTAMP}.log; do
    echo "--- $(basename $log) ---"
    grep -A 5 "LEAK SUMMARY" "$log" || echo "Nenhum vazamento encontrado"
    echo
done

echo "Resumo de erros encontrados:"
echo

for log in valgrind_logs/*_${TIMESTAMP}.log; do
    echo "--- $(basename $log) ---"
    grep "ERROR SUMMARY" "$log" || echo "Nenhum erro encontrado"
    echo
done

echo "========================================================================="
echo "  TESTES CONCLUÍDOS"
echo "========================================================================="
echo
echo "Logs salvos em: valgrind_logs/"
echo
echo "Para visualizar um log específico:"
echo "  less valgrind_logs/leak_nozend_${TIMESTAMP}.log"
echo
echo "Para buscar por vazamentos no log:"
echo "  grep -A 10 'definitely lost' valgrind_logs/leak_nozend_${TIMESTAMP}.log"
echo
