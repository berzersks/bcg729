#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_bcg729.h"

#include <stdint.h>
#include <string.h>
#include <zend_smart_string.h>
#include "bcg729/decoder.h"

// Argumentos
ZEND_BEGIN_ARG_INFO(arginfo_bcg729_hello, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729FrameToUlaw, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

// Função hello
ZEND_FUNCTION(bcg729_hello)
{
    RETURN_STRING("Hello from bcg729 extension!");
}

// Conversão PCM linear (int16_t) → µ-law (uint8_t)
static uint8_t linear_to_ulaw(int16_t pcm_val)
{
    const uint16_t BIAS = 0x84;
    const uint16_t CLIP = 32635;

    int sign = (pcm_val >> 8) & 0x80;
    if (sign)
        pcm_val = -pcm_val;
    if (pcm_val > CLIP)
        pcm_val = CLIP;
    pcm_val += BIAS;

    int exponent = 7;
    for (int expMask = 0x4000; (pcm_val & expMask) == 0 && exponent > 0; expMask >>= 1)
        exponent--;

    int mantissa = (pcm_val >> (exponent + 3)) & 0x0F;
    uint8_t ulaw = ~(sign | (exponent << 4) | mantissa);

    return ulaw;
}

// g729FrameToUlaw(string $payload): string
ZEND_FUNCTION(g729FrameToUlaw)
{
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE)
        RETURN_FALSE;

    if (input_len % 10 != 0) {
        php_error_docref(NULL, E_WARNING, "Expected G.729 payload with multiple of 10 bytes.");
        RETURN_FALSE;
    }

    size_t frames = input_len / 10;
    smart_string ulaw_result = {0};

    bcg729DecoderChannelContextStruct *decoder = initBcg729DecoderChannel();
    if (!decoder) {
        php_error_docref(NULL, E_WARNING, "Failed to initialize decoder.");
        RETURN_FALSE;
    }

    for (size_t i = 0; i < frames; i++) {
        const uint8_t *g729_payload = (const uint8_t *)(input + i * 10);
        int16_t pcmOut[80] = {0};

        bcg729Decoder(decoder, g729_payload, 0, 0, 0, 0, pcmOut);

        for (int j = 0; j < 80; j++) {
            smart_string_appendc(&ulaw_result, linear_to_ulaw(pcmOut[j]));
        }
    }

    closeBcg729DecoderChannel(decoder);
    smart_string_0(&ulaw_result);
    RETURN_STRINGL(ulaw_result.c, ulaw_result.len);
}

// Lista de funções expostas ao PHP
const zend_function_entry bcg729_functions[] = {
    PHP_FE(bcg729_hello, arginfo_bcg729_hello)
    PHP_FE(g729FrameToUlaw, arginfo_g729FrameToUlaw)
    PHP_FE_END
};

// Registro do módulo
zend_module_entry bcg729_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_BCG729_EXTNAME,
    bcg729_functions,
    NULL, NULL, NULL, NULL, NULL,
    PHP_BCG729_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BCG729
ZEND_GET_MODULE(bcg729)
#endif
