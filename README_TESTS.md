# Testes de Vazamento de Memória - bcg729

Este diretório contém scripts para testar e detectar vazamentos de memória na extensão bcg729.

## Scripts Disponíveis

### 1. test_memory_leak.php
Script PHP completo que testa todas as funções da extensão com milhares de iterações e monitora o uso de memória.

**Características:**
- Testa encode/decode do bcg729Channel
- Testa conversões PCMA/PCMU
- Testa conversões L16 (endianness)
- Testa mixagem de canais
- Testa múltiplas instâncias simultâneas
- Testa processamento de grandes volumes
- Gera relatório visual com gráficos de consumo

**Uso:**
```bash
php test_memory_leak.php
```

### 2. test_simple.php
Versão simplificada e rápida para testes básicos.

**Uso:**
```bash
php test_simple.php
```

### 3. test_valgrind.sh
Script automatizado para executar testes com Valgrind e detectar vazamentos precisos.

**Requisitos:**
```bash
# Ubuntu/Debian
sudo apt-get install valgrind

# Fedora/RHEL
sudo dnf install valgrind
```

**Uso:**
```bash
./test_valgrind.sh
```

**Saída:**
- Logs detalhados em `valgrind_logs/`
- Resumo de vazamentos e erros de memória
- Três tipos de análise: full, nozend, memcheck

## Como Interpretar os Resultados

### Teste PHP (test_memory_leak.php)

O crescimento de memória após GC indica possível vazamento:

- **< 100 KB**: Normal, sem vazamento
- **100 KB - 1 MB**: Pequeno crescimento, investigar
- **> 1 MB**: Vazamento provável, correção necessária

### Valgrind

Procure por estas seções nos logs:

#### 1. LEAK SUMMARY
```
LEAK SUMMARY:
   definitely lost: 0 bytes in 0 blocks
   indirectly lost: 0 bytes in 0 blocks
   possibly lost: 0 bytes in 0 blocks
   still reachable: 1,234 bytes in 5 blocks
   suppressed: 0 bytes in 0 blocks
```

- **definitely lost**: Vazamento confirmado (CRÍTICO)
- **indirectly lost**: Vazamento indireto (CRÍTICO)
- **possibly lost**: Possível vazamento (INVESTIGAR)
- **still reachable**: Memória alocada mas acessível (OK em muitos casos)

#### 2. ERROR SUMMARY
```
ERROR SUMMARY: 0 errors from 0 contexts
```

- **0 errors**: Perfeito, nenhum erro
- **> 0 errors**: Há problemas de memória

#### 3. Invalid read/write
```
Invalid read of size 4
   at 0x1234: function_name (file.c:123)
```

Indica acesso a memória inválida (buffer overflow, use-after-free, etc.)

## Exemplos de Uso

### Teste Rápido
```bash
# Teste básico de 5 minutos
php test_simple.php
```

### Teste Completo
```bash
# Teste completo de 15-30 minutos
php test_memory_leak.php
```

### Análise com Valgrind (Recomendado)
```bash
# Análise profunda (pode levar 30-60 minutos)
./test_valgrind.sh

# Ver resultados
ls -lh valgrind_logs/

# Ver vazamentos definitivos
grep -A 10 "definitely lost" valgrind_logs/leak_nozend_*.log
```

### Teste Manual com Valgrind
```bash
# Teste básico
valgrind --leak-check=full php test_simple.php

# Teste detalhado (mais lento mas mais preciso)
USE_ZEND_ALLOC=0 valgrind --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  php test_simple.php
```

## Correções Já Aplicadas

As seguintes correções foram aplicadas para prevenir vazamentos:

1. ✅ Remoção de variável `len` indefinida em decode/encode
2. ✅ Verificação de ponteiros NULL antes de usar decoder/encoder
3. ✅ Validação de inicialização de codecs no construtor
4. ✅ Eliminação de arrays temporários em loops
5. ✅ Pré-alocação de buffers para evitar realocações
6. ✅ Remoção de gc_collect_cycles() desnecessário
7. ✅ Validação de tamanhos antes de acessos a arrays

## Problemas Conhecidos

Nenhum vazamento conhecido após as correções aplicadas.

## Reporting Issues

Se você detectar vazamentos de memória:

1. Execute `./test_valgrind.sh`
2. Salve os logs gerados em `valgrind_logs/`
3. Identifique o tipo de vazamento
4. Documente os passos para reproduzir
5. Reporte com os logs anexados

## Performance Esperada

Após as otimizações:

- **Encode/Decode**: < 10 KB de crescimento após 10.000 iterações
- **Conversões**: < 5 KB por 10.000 iterações
- **Mixagem**: < 100 KB por 1.000 iterações (operação mais pesada)
- **Múltiplas instâncias**: < 50 KB por 10.000 instâncias criadas/destruídas

## Ferramentas Adicionais

### Massif (profiling de heap)
```bash
valgrind --tool=massif php test_memory_leak.php
ms_print massif.out.*
```

### Callgrind (profiling de performance)
```bash
valgrind --tool=callgrind php test_memory_leak.php
kcachegrind callgrind.out.*
```

### GDB (debugging interativo)
```bash
gdb php
(gdb) run test_simple.php
```
