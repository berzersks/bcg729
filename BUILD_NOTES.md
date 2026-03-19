# Build Notes - Extensão BCG729

## Sobre
Extensão PHP para codec de áudio G.729 usando a biblioteca bcg729.

## Dependências

### Bibliotecas Obrigatórias
- **libbcg729** (>= 1.0): Implementação do codec G.729

### Headers PHP Necessários
- `php.h`
- `php_bcg729.h`
- `zend_smart_string.h` - Para manipulação eficiente de strings

## Problema Corrigido

### Erro: undefined reference to `smart_string_*`
**Causa**: Funções `smart_string_alloc`, `smart_string_appendl`, etc. não eram encontradas
**Motivo**: Header `zend_smart_string.h` não estava incluído
**Solução**: Adicionar `#include "zend_smart_string.h"` no início do arquivo bcg729.c

## Compilação

```bash
# Instalar dependências (Ubuntu/Debian)
sudo apt-get install libbcg729-dev

# Compilar extensão
phpize
./configure --enable-bcg729
make
sudo make install
```

## Funções da API

### Classe BCG729Channel
- `__construct()` - Cria novo canal de codec
- `encode(string $pcm)` - Codifica PCM para G.729
- `decode(string $g729)` - Decodifica G.729 para PCM
- `destroy()` - Libera recursos do canal

### Formato de Áudio
- **Sample Rate**: 8000 Hz
- **Channels**: Mono (1 canal)
- **Format**: 16-bit signed PCM (little-endian)
- **Frame Size**: 10ms (80 samples = 160 bytes)
