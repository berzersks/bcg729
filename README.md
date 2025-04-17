# 🔊 Extensão PHP `bcg729` — G.729 Codec Nativo em PHP

Esta extensão fornece funções nativas para **codificar e decodificar áudio G.729** diretamente em PHP, utilizando a biblioteca `bcg729` desenvolvida pela [Belledonne Communications](https://github.com/BelledonneCommunications/bcg729).

Ideal para aplicações VoIP, proxies RTP, media servers ou sistemas de gravação/transcodificação SIP que precisam de performance e baixo overhead, sem dependência de daemons externos.

---

## 🚀 Instalação

### Pré-requisitos

- PHP >= 8.0
- Biblioteca `bcg729` compilada estaticamente (linkada)
- Ambiente de build C (`gcc`, `make`, etc.)

### Build Manual (clássico)

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

Decodifica uma stream binária G.729 em múltiplos blocos PCM (16-bit, 80 amostras por frame).

**Parâmetros:**

- `$input`: string binária contendo **múltiplos de 10 bytes** (cada 10B = 10ms G.729)

**Retorno:**
- `array` com strings binárias (cada uma com 160 bytes = 80 samples PCM)

**Exemplo:**

```php
<?php

function generateWavHeaderULAW(int $dataLength, int $sampleRate = 8000, int $channels = 1): string
{
    $byteRate = $sampleRate * $channels * 1;  // 1 byte por amostra
    $blockAlign = $channels * 1;

    return pack('A4V', 'RIFF', 36 + $dataLength)
        . 'WAVE'
        . pack('A4VvvVVvv', 'fmt ', 16, 7, $channels, $sampleRate, $byteRate, $blockAlign, 8) 
        . pack('A4V', 'data', $dataLength);
}

function linearToUlaw(int $sample): int
{
    // Implementação clássica ITU-T G.711 µ-law
    $MAX = 0x7FFF;
    $BIAS = 0x84;

    $sign = ($sample >> 8) & 0x80;
    if ($sign !== 0) {
        $sample = -$sample;
    }
    $sample = min($sample + $BIAS, $MAX);

    $exponent = 7;
    $expMask = 0x4000;
    while (($sample & $expMask) === 0 && $exponent > 0) {
        $expMask >>= 1;
        $exponent--;
    }

    $mantissa = ($sample >> (($exponent === 0) ? 4 : ($exponent + 3))) & 0x0F;
    $ulawByte = ~($sign | ($exponent << 4) | $mantissa) & 0xFF;

    return $ulawByte;
}

$inputG729 = 'file.g729';
$outputWav = 'convertido_com_php.wav';

$g729 = file_get_contents($inputG729);
if (!$g729 || strlen($g729) < 10) {
    die("Arquivo G.729 inválido ou muito pequeno.\n");
}

$g729 = substr($g729, 0, intdiv(strlen($g729), 10) * 10);
$frames = bcg729DecodeStream($g729);
if (!is_array($frames)) {
    die("Erro ao decodificar com bcg729DecodeStream()\n");
}

$ulaw = '';
foreach ($frames as $frame) {
    for ($i = 0; $i < strlen($frame); $i += 2) {
        $sample = unpack('s', substr($frame, $i, 2))[1];
        $ulaw .= chr(linearToUlaw($sample));
    }
}

$wav = generateWavHeaderULAW(strlen($ulaw)) . $ulaw;
file_put_contents($outputWav, $wav);

echo "✅ WAV µ-law gerado em: \"$outputWav\" (" . strlen($ulaw) . " bytes de áudio)\n";

```

---

### `bcg729EncodeStream(string $input): array`

Codifica uma stream PCM 16-bit (160 bytes por frame = 10ms) para frames G.729.

**Parâmetros:**

- `$input`: string binária com múltiplos de **160 bytes** (80 samples por frame)

**Retorno:**
- `array` de arrays, cada um com:
  - `output`: string binária G.729 (normalmente 10 bytes)
  - `length`: tamanho real do frame (em bytes)

**Exemplo:**

```php
$pcm = file_get_contents('audio.raw'); // múltiplos de 160 bytes
$g729Frames = bcg729EncodeStream($pcm);

foreach ($g729Frames as $frame) {
    file_put_contents('audio.g729', $frame['output'], FILE_APPEND);
}
```

---

## 🎙️ Aplicações típicas

- RTP Proxy com transcodificação G.729 ↔ PCMU (G.711)
- Gravação de chamadas VoIP com compressão G.729
- Conversão de áudios para formato leve para transporte
- Sistemas de voz com foco em economia de banda

---

## 📜 Licença

Esta extensão usa a biblioteca [`bcg729`](https://github.com/BelledonneCommunications/bcg729), licenciada sob **GPLv3**.  
Distribuição e uso devem respeitar os termos da GPLv3.

> Este projeto é um wrapper em PHP sobre a biblioteca original `bcg729`, criada e mantida por Belledonne Communications.

---

## 🙌 Contribuições

Pull requests, issues e melhorias são bem-vindas!

---

---
