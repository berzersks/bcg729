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

## 🎧 Exemplo Completo

```php
function generateWavHeaderUlaw(int $dataLength, int $sampleRate = 8000, int $channels = 1): string {
    $byteRate = $sampleRate * $channels;
    $blockAlign = $channels;

    return pack('A4V', 'RIFF', 36 + $dataLength)
        . 'WAVE'
        . pack('A4VvvVVvv', 'fmt ', 16, 7, $channels, $sampleRate, $byteRate, $blockAlign, 8)
        . pack('A4V', 'data', $dataLength);
}

function searchSegment(int $val, array $table): int {
    foreach ($table as $i => $v) {
        if ($val <= $v) return $i;
    }
    return count($table);
}

function linear2ulaw(int $pcm_val): int {
    $BIAS = 0x84;
    $seg_end = [0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF];

    if ($pcm_val < 0) {
        $pcm_val = $BIAS - $pcm_val;
        $mask = 0x7F;
    } else {
        $pcm_val += $BIAS;
        $mask = 0xFF;
    }

    $seg = searchSegment($pcm_val, $seg_end);
    if ($seg >= 8) return (0x7F ^ $mask);

    $uval = ($seg << 4) | (($pcm_val >> ($seg + 3)) & 0xF);
    return $uval ^ $mask;
}

$inputG729 = 'file.pcm';
$outputUlaw = 'convertido_ulaw.wav';
$outputG729 = 'reconvertido_g729.raw';
$inputG729Final = 'audio.g729.wav';

$g729 = file_get_contents($inputG729);
if (!$g729 || strlen($g729) < 10) die("Arquivo G.729 inválido.\n");
$g729 = substr($g729, 0, intdiv(strlen($g729), 10) * 10);
$frames = bcg729DecodeStream($g729);
if (!is_array($frames)) die("Erro ao decodificar.\n");

$ulaw = '';
$pcmTotal = '';
foreach ($frames as $frame) {
    for ($i = 0; $i < strlen($frame); $i += 2) {
        $sample = unpack('v', substr($frame, $i, 2))[1];
        if ($sample > 32767) $sample -= 65536;
        $pcmTotal .= pack('s', $sample);
        $ulaw .= chr(linear2ulaw($sample));
    }
}

$wav = generateWavHeaderUlaw(strlen($ulaw)) . $ulaw;
file_put_contents($outputUlaw, $wav);
echo "✅ WAV µ-law gerado: $outputUlaw\n";

$g729Encoded = bcg729EncodeStream($pcmTotal);
if (!is_array($g729Encoded)) die("Erro ao reencodar.\n");
$output = '';
foreach ($g729Encoded as $chunk) {
    $output .= $chunk['output'];
}
file_put_contents($outputG729, $output);
echo "✅ RAW G.729 salvo: $outputG729\n";

$g729Final = file_get_contents($outputG729);
$g729Final = substr($g729Final, 0, intdiv(strlen($g729Final), 10) * 10);
$framesFinal = bcg729DecodeStream($g729Final);
$ulawFinal = '';
foreach ($framesFinal as $frame) {
    for ($i = 0; $i < strlen($frame); $i += 2) {
        $sample = unpack('v', substr($frame, $i, 2))[1];
        if ($sample > 32767) $sample -= 65536;
        $ulawFinal .= chr(linear2ulaw($sample));
    }
}
$wavFinal = generateWavHeaderUlaw(strlen($ulawFinal)) . $ulawFinal;
file_put_contents($inputG729Final, $wavFinal);
echo "✅ WAV µ-law final gerado: $inputG729Final\n";
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

