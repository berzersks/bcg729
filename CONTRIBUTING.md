Contribuindo para bcg729 (Extensão PHP)

Obrigado por considerar contribuir! Este projeto utiliza o fluxo de build via phpize. Veja abaixo o resumo para começar.

Como começar

1. phpize
2. ./configure --enable-bcg729
3. make -j
4. Teste local: php -d extension=$(pwd)/modules/bcg729.so -m | grep bcg729

Testes

- Rápido: php test_simple.php
- Completo: php test_memory_leak.php
- Valgrind: ./test_valgrind.sh

Padrões

- Siga o estilo do código existente (C / Zend API)
- Mensagens de commit claras, no imperativo

Fluxo de PR

1. Crie um branch (feat/.., fix/..)
2. Garanta build e testes OK
3. Abra o PR descrevendo contexto, mudanças e validação

Segurança e Conduta

- Reporte vulnerabilidades em SECURITY.md
- Siga o CODE_OF_CONDUCT.md

Licença
Ao contribuir, você concorda com a GPLv3 do projeto (ver LICENSE).
