# 🔊 Extensão PHP `bcg729` — Codec G.729 Nativo em PHP

Esta extensão fornece funções nativas para **codificar e decodificar áudio G.729** diretamente em PHP, utilizando a biblioteca [bcg729](https://github.com/BelledonneCommunications/bcg729) da Belledonne Communications.

Ideal para aplicações VoIP, proxies RTP, media servers ou sistemas de gravação/transcodificação SIP que precisam de performance e baixo overhead, sem dependência de daemons externos.

---

## 🚀 Instalação

### Requisitos

- PHP >= 8.0
- Biblioteca `bcg729` compilada estaticamente (linkada)
- Ambiente de build C (gcc, make, etc.)

### Build Manual

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

---

## ✨ Funções Disponíveis

### `bcg729DecodeStream(string $input): array`

Decodifica uma stream binária G.729 em blocos PCM (16-bit, 80 amostras por frame).

**Parâmetros:**

- `$input`: string binária com múltiplos de 10 bytes (cada frame = 10ms G.729)

**Retorno:**

- `array` com strings binárias de 160 bytes cada (80 samples PCM)

### `bcg729EncodeStream(string $input): array`

Codifica uma stream PCM 16-bit (160 bytes por frame) para frames G.729.

**Parâmetros:**

- `$input`: string binária com múltiplos de 160 bytes (80 samples por frame)

**Retorno:**

- `array` de arrays com:
  - `output`: frame G.729 (normalmente 10 bytes)
  - `length`: tamanho do frame em bytes

---

## 🎧 Stubs de ajuda
```php
class bcg729Channel {
    public function __construct() {}

    public function decode(string $input): mixed  {}

    public function encode(string $input): mixed  {}

    public function info() {}

    public function close() {}

}
```

---

## 🌜 Aplicações

- Proxy RTP com transcodificação G.729 ↔ PCMU (G.711)
- Gravação de chamadas VoIP com compressão
- Transformação de áudio para economia de banda

---

## 📄 Licença

Esta extensão utiliza a biblioteca [bcg729](https://github.com/BelledonneCommunications/bcg729) sob **GPLv3**.

> Projeto wrapper PHP baseado na biblioteca original mantida por Belledonne Communications.

---

## 🙌 Contribuições

Pull requests e melhorias são bem-vindos!

