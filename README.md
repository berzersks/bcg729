# 🔊 Extensão PHP `bcg729` — Codec G.729 nativo para PHP

Extensão em C para PHP que expõe primitivas para **codificar e decodificar áudio G.729** e utilitários de áudio
relacionados, baseada na biblioteca upstream [bcg729](https://github.com/BelledonneCommunications/bcg729) (Belledonne
Communications).

Ideal para aplicações VoIP, proxies RTP, media servers ou sistemas de gravação/transcodificação SIP que precisam de alta
performance sem daemons externos.

—

## 🧰 Stack e pontos de entrada

- Linguagem/stack: C (extensão para PHP 8), Zend API
- Geradores de build: Autotools via `phpize`/`configure` (primário); há um `CMakeLists.txt` auxiliar (uso para IDEs) —
  TODO: documentar suporte oficial a CMake se aplicável
- Biblioteca nativa requerida: `bcg729` (headers e lib `-lbcg729`)
- Módulo PHP: `bcg729`
- Ponto de entrada (PHP):
    - Classe `bcg729Channel` com métodos `__construct`, `decode`, `encode`, `info`, `close`
    - Funções globais:
        - `decodePcmaToPcm(string $input): string`
        - `decodePcmuToPcm(string $input): string`
        - `encodePcmToPcma(string $input): string`
        - `encodePcmToPcmu(string $input): string`
        - `decodeL16ToPcm(string $input): string`
        - `encodePcmToL16(string $input): string`
        - `mixAudioChannels(array $channels, int $sample_rate): string`
        - `pcmLeToBe(string $input): string`

—

## ✅ Requisitos

- PHP 8.x com ferramentas de desenvolvimento (`phpize`, headers)
- Compilador e ferramentas: `gcc`, `make`, `autoconf`
- Biblioteca nativa [bcg729] instalada (headers e biblioteca compartilhada/estática)
    - TODO: listar nomes de pacotes por distribuição (ex.: Debian/Ubuntu `libbcg729-dev`?)

—

## 🚀 Instalação (Autotools/phpize)

```bash
phpize
./configure
make
sudo make install
```

Ative no seu `php.ini`:

```ini
extension=bcg729
```

Verifique o carregamento:

```bash
php -m | grep bcg729
```

Observações:

- O arquivo `config.m4` liga contra `-lbcg729`. Garanta que o linker encontre a biblioteca (ex.: ajuste
  `LD_LIBRARY_PATH` ou instale no prefixo padrão do sistema).
- Linkagem estática vs dinâmica dependerá do ambiente e de como `bcg729` foi instalado. TODO: documentar cenário de
  linkagem estática, se suportado/necessário.

—

## 🧪 Testes

O repositório inclui `run-tests.php` (harness padrão do PHP).

Formas comuns de executar:

- Via `make test` após compilar:

```bash
make test
```

- Ou manualmente indicando o binário do PHP:

```bash
php -d extension=bcg729 run-tests.php -p "$(which php)"
```

—

## 🎧 Uso rápido (API)

G.729 opera com quadros de 10 ms: 10 bytes por quadro no bitstream e 80 amostras PCM16 (mono, 8 kHz), ou seja, 160 bytes
por quadro em PCM16LE.

### Classe `bcg729Channel`

```php
$ch = new bcg729Channel();

// Decodifica bitstream G.729 -> PCM16LE (@8kHz mono)
// Entrada deve ter tamanho múltiplo de 10 bytes
$pcm = $ch->decode($g729Bytes);

// Codifica PCM16LE (@8kHz mono) -> G.729
// Entrada deve ter tamanho múltiplo de 160 bytes (80 samples * 2 bytes)
$g729 = $ch->encode($pcmBytes);

$info = $ch->info();   // [decoder_initialized => bool, encoder_initialized => bool]
$ch->close();          // Libera recursos (retorna true)
```

### Funções utilitárias

```php
// Lei A/µ <-> PCM16
$pcm   = decodePcmaToPcm($alawBytes);
$pcm   = decodePcmuToPcm($ulawBytes);
$alaw  = encodePcmToPcma($pcmBytes);
$ulaw  = encodePcmToPcmu($pcmBytes);

// PCM16 big-endian <-> little-endian
$pcmBe = encodePcmToL16($pcmLe);
$pcmLe = decodeL16ToPcm($pcmBe);

// Mix de múltiplos canais PCM16LE em uma única trilha
$mix = mixAudioChannels([$ch1Bytes, $ch2Bytes, /* ... */], 8000);

// Conversão LE -> BE direta
$be = pcmLeToBe($le);
```

Validações de tamanho na implementação:

- `bcg729Channel::decode` retorna `false` se o tamanho de entrada não for múltiplo de 10 bytes.
- `bcg729Channel::encode` retorna `false` se o tamanho de entrada não for múltiplo de 160 bytes.

—

## 📦 Scripts e comandos úteis

- `phpize` / `./configure` / `make` / `make install`: ciclo padrão de build/instalação
- `make test` ou `run-tests.php`: execução da suíte de testes padrão de extensões PHP

—

## 🔐 Variáveis de ambiente / Configuração

Nenhuma configuração INI própria é exposta no momento. O módulo é carregado como `extension=bcg729`.

Possíveis variáveis do ambiente de build (dependem do sistema):

- `PKG_CONFIG_PATH`, `CFLAGS`, `LDFLAGS` — caso precise apontar para onde a `bcg729` está instalada. TODO: adicionar
  exemplos por plataforma.

—

## 🗂️ Estrutura do projeto

```
.
├── bcg729.c            # Implementação da extensão (classe, funções e hooks do módulo)
├── php_bcg729.h        # Header da extensão (nome, versão, entry)
├── config.m4           # Configuração para phpize/autoconf, ligação com -lbcg729
├── configure.ac        # Autotools (gerado/necessário ao configure)
├── run-tests.php       # Harness de testes das extensões PHP
├── CMakeLists.txt      # Arquivo CMake (auxiliar/IDE) — TODO: confirmar suporte oficial
├── LICENSE             # Licença do projeto
├── README.md           # Este documento
└── SECURITY.md         # Política de segurança
```

—

## 🌜 Casos de uso

- Proxy RTP com transcodificação G.729 ↔ PCMU (G.711)
- Gravação de chamadas VoIP com compressão
- Transcodificação para economia de banda

—

## 📄 Licença

Este projeto está licenciado sob **GNU GPL v2** (ver arquivo `LICENSE`).

ATENÇÃO: a biblioteca upstream [bcg729](https://github.com/BelledonneCommunications/bcg729) tem sua própria licença;
verifique compatibilidade para o seu uso.

—

## 🤝 Contribuições

Pull requests e melhorias são bem-vindos! Antes de submeter, rode os testes e siga o estilo do código existente.

—

## 📝 TODOs

- Documentar pacotes por distribuição para `bcg729` (ex.: Debian/Ubuntu/Fedora/Alpine)
- Confirmar e documentar suporte oficial ao build via CMake (atualmente o caminho suportado é `phpize`)
- Exemplos completos de fluxo RTP (I/O de quadros) e integração com streams

