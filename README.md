# bcg729 - Extensão PHP para decodificação G.729

Esta extensão fornece decodificação de frames G.729 em tempo real diretamente no PHP, com suporte à conversão para áudio µ-law (G.711). Ideal para sistemas de telefonia VoIP, gravações SIP, proxies RTP ou discadoras.

## 🔧 Instalação

### Pré-requisitos

- PHP >= 7.4 (compatível com PHP 8.3+)
- Biblioteca `bcg729` compilada como estática (`libbcg729.a`)
- `phpize` e headers do PHP instalados

### Passos

```bash
phpize
./configure --enable-bcg729
make -j$(nproc)
sudo make install
```

## 🧠 bcg729.stub.php

```php
<?php
function bcg729_hello(): string {}

/**
 * Converte um ou mais frames G.729 (10 bytes por frame) para µ-law (80 bytes por frame).
 *
 * @param string $input Payload G.729 puro, múltiplos de 10 bytes
 * @return string Áudio convertido em µ-law
 */
function g729FrameToUlaw(string $input): string {}
```

## 📜 Licença
Esta extensão é licenciada sob a Licença MIT. Consulte o arquivo LICENSE para mais detalhes.
