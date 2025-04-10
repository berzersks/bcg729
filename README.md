# bcg729 - Extensão PHP para decodificação G.729

Modulo PHP para conversão e decodificação de pacotes G729.

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
function bcg729decodestream(string $input): array {
}

function bcg729encodestream(string $input): array {
}
```

## 📜 Licença
Esta extensão é licenciada sob a Licença MIT. Consulte o arquivo LICENSE para mais detalhes.
