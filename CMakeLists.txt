cmake_minimum_required(VERSION 3.16)

# Observação importante:
# O fluxo de build oficial desta extensão é via phpize/autoconf (config.m4).
# Este CMakeLists.txt é apenas experimental para futuros estudos e fica
# desativado por padrão para evitar confusões.

project(bcg729 LANGUAGES C)

option(BUILD_EXPERIMENTAL_CMAKE "Habilita build experimental via CMake (NÃO SUPORTADO)." OFF)

if (NOT BUILD_EXPERIMENTAL_CMAKE)
    message(STATUS "CMake experimental está desativado. Use phpize: phpize && ./configure --enable-bcg729 && make")
    return()
endif()

message(WARNING "Você habilitou o build experimental via CMake. O caminho SUPORTADO é phpize. Este fluxo pode falhar.")

# Tente localizar headers do PHP, se o usuário quiser insistir no build experimental
set(PHP_INCLUDE_DIR "" CACHE PATH "Caminho para includes do PHP (ex.: /usr/include/php/20230831)")
if (NOT PHP_INCLUDE_DIR)
    message(FATAL_ERROR "PHP_INCLUDE_DIR não definido. Ex.: -DPHP_INCLUDE_DIR=/usr/include/php/20230831. Use phpize para o build suportado.")
endif()

include_directories(
    ${PHP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Como prova de conceito apenas criamos uma biblioteca estática com o código.
# Isso NÃO gera a extensão carregável por PHP corretamente.
add_library(bcg729_experimental STATIC
    bcg729.c
    php_bcg729.h
)

target_compile_definitions(bcg729_experimental PRIVATE ZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
