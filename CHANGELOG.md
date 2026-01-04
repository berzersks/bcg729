# Changelog

Todas as mudanças notáveis deste projeto serão documentadas neste arquivo.

O formato segue o padrão Keep a Changelog e este projeto adota Versionamento Semântico (SemVer) quando aplicável.

## [Unreleased]

### Adicionado

- Documentação profissional: `CONTRIBUTING.md`, `CHANGELOG.md`, templates de Issue e PR.
- Sumário no `README.md` e instruções para o CMake experimental.
- CMakeLists revisado para ser opcional e desativado por padrão.

### Alterado

- Melhoria na organização do repositório e orientação de build (preferir `phpize`).

### Correções

- Evita confusão com CMake incorreto adicionando mensagens e opção de build experimental.

## [0.1.0] - 2025-11-29

### Adicionado

- Extensão PHP `bcg729` inicial (encode/decode G.729, utilitários PCMA/PCMU/L16, mixagem).
- Scripts de teste: `test_simple.php`, `test_memory_leak.php`, `test_valgrind.sh` e `README_TESTS.md`.
- Arquivos de build via `phpize`: `config.m4`, `configure.ac` etc.

[Unreleased]: https://example.com/compare/v0.1.0...HEAD

[0.1.0]: https://example.com/releases/tag/v0.1.0
