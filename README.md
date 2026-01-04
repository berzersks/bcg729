# 🔊 Extensão PHP `bcg729` — Codec G.729 nativo em PHP

Extensão PHP escrita em C que expõe funções e uma classe para **codificar/decodificar áudio G.729**, além de utilitários
de conversão de áudio (A-law/μ-law/L16, mixagem), utilizando a biblioteca
nativa [bcg729](https://github.com/BelledonneCommunications/bcg729) da Belledonne Communications.

Ideal para aplicações VoIP (SIP/RTP), proxies de mídia, media servers e sistemas de gravação/transcodificação que
precisam de baixo overhead e alta performance sem serviços externos.

—

## 📑 Sumário

- [Stack / Detecção do projeto](#-stack--detecção-do-projeto)
- [Visão geral do que é exposto](#-visão-geral-do-que-é-exposto)
- [Requisitos](#-requisitos)
- [Instalação (via phpize)](#-instalação-via-phpize)
- [Como executar os testes / scripts incluídos](#-como-executar-os-testes--scripts-incluídos)
- [Variáveis de ambiente úteis](#-variáveis-de-ambiente-úteis)
- [Exemplos rápidos de uso](#-exemplos-rápidos-de-uso)
- [Estrutura do projeto (resumo)](#-estrutura-do-projeto-resumo)
- [Notas de build e scripts](#-notas-de-build-e-scripts)
- [Licença](#-licença)
- [Contribuições](#-contribuições)
  - [Guia de Contribuição](#guia-de-contribuição)
  - [Changelog](#changelog)

## 📦 Stack / Detecção do projeto

- Linguagem principal: C (extensão PHP via Zend API)
- Runtime alvo: PHP 8.x
- Sistema de build principal: `phpize` + `autoconf`/`make` (via `config.m4`)
- Dependência nativa: biblioteca `bcg729` (linkada como `-lbcg729`)
- Alternativo (experimental): arquivo `CMakeLists.txt` presente, porém não recomendado no momento (ver Nota/TODO abaixo)

—

## 🧭 Visão geral do que é exposto

### Classe `bcg729Channel`

- `__construct()` — cria um canal com encoder/decoder G.729
- `encode(string $pcm16le): string` — codifica PCM 16‑bit LE em G.729
- `decode(string $g729): string` — decodifica G.729 para PCM 16‑bit LE
- `info(): array|mixed` — informações do canal (implem.)
- `close(): void` — libera recursos nativos

### Funções auxiliares (globais)

- `encodePcmToPcma(string $pcm16le): string` — PCM 16‑bit LE → A‑law
- `encodePcmToPcmu(string $pcm16le): string` — PCM 16‑bit LE → μ‑law
- `decodePcmaToPcm(string $pcma): string` — A‑law → PCM 16‑bit LE
- `decodePcmuToPcm(string $pcmu): string` — μ‑law → PCM 16‑bit LE
- `encodePcmToL16(string $pcm16le_be?): string` e `decodeL16ToPcm(string $l16_be): string` — conversões L16/endianness
- `mixAudioChannels(array $frames, int $sampleRate): string` — mixagem simples de canais PCM
- `pcmLeToBe(string $pcm16le): string` — utilitário de endianness

Observação: os nomes/assinaturas acima foram extraídos do código fonte (`bcg729.c`). Para detalhes exatos consulte o
arquivo.

—

## ✅ Requisitos

- PHP 8.0+ com headers de desenvolvimento (ex.: `php-dev`/`php-devel`)
- Ferramentas de build: `phpize`, `autoconf`, `make`, `gcc`
- Biblioteca nativa `bcg729` disponível no sistema para linkagem (`-lbcg729`)
  - TODO: documentar passos oficiais de instalação para cada distro (Debian/Ubuntu/Fedora/macOS). Em muitas distros o
    pacote chama algo como `libbcg729-dev`/`bcg729`.

—

## 🚀 Instalação (via phpize)

```bash
# 1) Preparar o ambiente de build
phpize

# 2) Configurar (passe --with-php-config se necessário)
./configure --enable-bcg729

# 3) Compilar e instalar a extensão
make
sudo make install

# 4) Habilitar a extensão (php.ini ou conf.d)
echo "extension=bcg729" | sudo tee /etc/php/*/mods-available/bcg729.ini >/dev/null
sudo phpenmod bcg729 2>/dev/null || true
```

Verifique a instalação:

```bash
php -m | grep bcg729
php -r 'var_dump(class_exists("bcg729Channel"));'
```

—

## 🧪 Como executar os testes / scripts incluídos

Scripts de teste e diagnóstico (ver pasta raiz):

- `test_simple.php` — teste básico e rápido de encode/decode e utilitários
  - Execução: `php test_simple.php`

- `test_memory_leak.php` — teste mais completo com milhares de iterações e métricas de memória
  - Execução: `php test_memory_leak.php`

- `test_valgrind.sh` — integração com Valgrind para checar vazamentos (requer Valgrind)
  - Execução: `./test_valgrind.sh`
  - Saída: logs em `valgrind_logs/` (detalhes em `README_TESTS.md`)

- `demo_real_audio.php` — demonstração com áudio real (exemplos de uso de conversões)

Mais detalhes em `README_TESTS.md`.

—

## 🌿 Variáveis de ambiente úteis

- `USE_ZEND_ALLOC=0` — recomendado ao usar Valgrind para relatórios mais precisos
- `PHP_INI_SCAN_DIR` — para apontar um diretório com `bcg729.ini` customizado
- `PHPRC` — para carregar um `php.ini` específico durante testes

—

## 🧩 Exemplos rápidos de uso

```php
<?php

$ch = new bcg729Channel();

// PCM 16‑bit (LE) de 10 ms @ 8 kHz (80 samples):
$pcm = str_repeat("\x00\x00", 80);

$g729 = $ch->encode($pcm);
$back = $ch->decode($g729);

// Utilitários A‑law/μ‑law
$pcma = encodePcmToPcma($pcm);
$pcmu = encodePcmToPcmu($pcm);

$pcm_from_a = decodePcmaToPcm($pcma);
$pcm_from_u = decodePcmuToPcm($pcmu);

$ch->close();
```

—

## 🗂️ Estrutura do projeto (resumo)

```
.
├─ bcg729.c              # Implementação da extensão PHP (Zend API)
├─ php_bcg729.h          # Cabeçalho da extensão
├─ config.m4             # Configuração para phpize/autoconf
├─ configure.ac, configure, Makefile*  # Artefatos de build
├─ test_simple.php       # Teste rápido
├─ test_memory_leak.php  # Teste extensivo / métricas de memória
├─ test_valgrind.sh      # Script de Valgrind
├─ demo_real_audio.php   # Demo com áudio real
├─ README_TESTS.md       # Documentação dos testes
├─ CMakeLists.txt        # Build alternativo (experimental) — ver Nota
└─ LICENSE               # Licença (GPLv3)
```

—

## 🛠️ Notas de build e scripts

- `config.m4` declara `PHP_NEW_EXTENSION(bcg729, bcg729.c, $ext_shared)` e adiciona link com `bcg729` (
  `PHP_ADD_LIBRARY(bcg729, 1, bcg729)`). Certifique-se de que a lib nativa esteja instalada e visível ao linker (por
  exemplo, via `/usr/lib`, `pkg-config`, `LD_LIBRARY_PATH`, etc.).
- Makefiles presentes na raiz podem ser artefatos de builds anteriores. O caminho suportado é via `phpize` (se houver
  divergência, prefira regenerar com `phpize`).
- `CMakeLists.txt`: arquivo presente, porém inclui arquivos não‑fonte como dependências e requer ajustes. No momento o
  fluxo oficial é `phpize`.
  - TODO: Revisar/arrumar o build via CMake (versão mínima, fontes corretas, includes e linkagem do `bcg729`).

Observação: o CMake agora está desativado por padrão e marcado como experimental. Para forçar (não recomendado):

```bash
cmake -S . -B build -DBUILD_EXPERIMENTAL_CMAKE=ON -DPHP_INCLUDE_DIR=/usr/include/php/20230831
cmake --build build
```

—

## 📜 Licença

O projeto é licenciado sob **GPLv3** (ver arquivo `LICENSE`). A biblioteca
subjacente [bcg729](https://github.com/BelledonneCommunications/bcg729) também é GPLv3.

—

## 🤝 Contribuições

Contribuições são bem‑vindas! Sinta‑se à vontade para abrir issues, enviar PRs ou propor melhorias na
documentação/testes.

### Guia de Contribuição

Consulte `CONTRIBUTING.md` para setup do ambiente, estilo, testes e fluxo de PR.

### Changelog

Mudanças versionadas seguem o padrão Keep a Changelog. Veja `CHANGELOG.md`.

