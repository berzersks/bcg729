#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_bcg729.h"

#include <stdint.h>
#include <string.h>
#include <zend_smart_string.h>
#include "bcg729/decoder.h"
#include "bcg729/encoder.h"


// -------------------- ARGINFO --------------------

ZEND_BEGIN_ARG_INFO(arginfo_bcg729Decode, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_bcg729Encode, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

// -------------------- FUNÇÕES PHP --------------------
ZEND_FUNCTION(bcg729Decode) {
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE)
        RETURN_FALSE;

    if (input_len % 10 != 0) {
        php_error_docref(NULL, E_WARNING, "Expected G.729 payload to be a multiple of 10 bytes.");
        RETURN_FALSE;
    }

    size_t frames = input_len / 10;
    int16_t *outputFrames = (int16_t *) emalloc(frames * 80 * sizeof(int16_t)); // 80 amostras por frame

    smart_string pcm_result = {0}; // <- mover pra fora

    bcg729DecoderChannelContextStruct *decoder = initBcg729DecoderChannel();
    if (!decoder) {
        php_error_docref(NULL, E_WARNING, "Failed to initialize decoder.");
        efree(outputFrames);
        RETURN_FALSE;
    }

    for (size_t i = 0; i < frames; i++) {
        bcg729Decoder(
            decoder,
            (const uint8_t *)(input + i * 10),
            10,
            0, // frameErased
            0, // SIDFrameFlag
            0, // rfc3389PayloadFlag
            (int16_t *)(outputFrames + i * 80)
        );

        // adiciona 80 amostras (160 bytes) ao smart_string
        for (size_t sample_idx = 0; sample_idx < 80; sample_idx++) {
            uint16_t sample = (uint16_t) outputFrames[i * 80 + sample_idx];
            smart_string_appendc(&pcm_result, sample & 0xFF);         // byte baixo
            smart_string_appendc(&pcm_result, (sample >> 8) & 0xFF);  // byte alto
        }
    }

    closeBcg729DecoderChannel(decoder);
    efree(outputFrames);

    smart_string_0(&pcm_result);
    RETURN_STRINGL(pcm_result.c, pcm_result.len);
}


ZEND_FUNCTION(bcg729Encode) {
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE)
        RETURN_FALSE;

    if (input_len % 160 != 0) {
        php_error_docref(NULL, E_WARNING, "Expected PCM payload to be a multiple of 160 bytes.");
        RETURN_FALSE;
    }

    size_t frames = input_len / 160;
    smart_string g729_result = {0};

    bcg729EncoderChannelContextStruct *encoder = initBcg729EncoderChannel();
    if (!encoder) {
        php_error_docref(NULL, E_WARNING, "Failed to initialize encoder.");
        RETURN_FALSE;
    }

    for (size_t i = 0; i < frames; i++) {
        const int16_t *pcm_frame = (const int16_t *) (input + i * 160);
        uint8_t g729_out[10] = {0};
    }

    closeBcg729EncoderChannel(encoder);
    smart_string_0(&g729_result);
    RETURN_STRINGL(g729_result.c, g729_result.len);
}

// -------------------- REGISTRO DAS FUNÇÕES --------------------

const zend_function_entry bcg729_functions[] = {
    PHP_FE(bcg729Decode, arginfo_bcg729Decode)
    PHP_FE(bcg729Encode, arginfo_bcg729Encode)
    PHP_FE_END
};

// -------------------- REGISTRO DO MÓDULO --------------------

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
