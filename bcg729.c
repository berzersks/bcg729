#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_bcg729.h"

#include <stdint.h>
#include <string.h>
#include "bcg729/decoder.h"
#include "bcg729/encoder.h"
#include <zend_smart_string.h>

// -------------------- ARGINFO --------------------
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcg729DecodeStream, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcg729EncodeStream, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()




// -------------------- FUNÇÃO PRINCIPAL --------------------
ZEND_FUNCTION(bcg729DecodeStream) {
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (input_len % 10 != 0) {
        php_error_docref(NULL, E_WARNING,
                         "Tamanho inválido: stream deve conter múltiplos de 10 bytes (1 frame por chunk).");
        RETURN_FALSE;
    }

    bcg729DecoderChannelContextStruct *context = initBcg729DecoderChannel();
    if (!context) {
        php_error_docref(NULL, E_WARNING, "Falha ao inicializar decoder.");
        RETURN_FALSE;
    }

    array_init(return_value);
    size_t offset = 0;

    while (offset + 10 <= input_len) {
        const uint8_t *bitStream = (const uint8_t *) (input + offset);
        int16_t pcmOut[80] = {0};

        bcg729Decoder(
            context,
            bitStream,
            10, // bitStreamLength
            0, // frameErasureFlag
            0, // SIDFrameFlag
            0, // rfc3389PayloadFlag
            pcmOut
        );

        // Convertendo para string binária
        char *output = (char *) emalloc(160); // 80 * sizeof(int16_t)
        for (int i = 0; i < 80; i++) {
            output[i * 2] = pcmOut[i] & 0xFF;
            output[i * 2 + 1] = (pcmOut[i] >> 8) & 0xFF;
        }

        // Criando array com a saída
        zval frame_result;
        array_init(&frame_result);
        add_assoc_stringl(&frame_result, "output", output, 160);
        efree(output);

        add_next_index_zval(return_value, &frame_result);

        offset += 10;
    }
    closeBcg729DecoderChannel(context);
    // return value
    RETURN_ZVAL(return_value, 1, 0);
}

ZEND_FUNCTION(bcg729EncodeStream) {
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (input_len % 160 != 0) {
        php_error_docref(NULL, E_WARNING,
                         "Tamanho inválido: stream deve conter múltiplos de 160 bytes (1 frame por chunk).");
        RETURN_FALSE;
    }

    /*****************************************************************************/
    /* bcg729Encoder :                                                           */
    /*    parameters:                                                            */
    /*      -(i) encoderChannelContext : context for this encoder channel        */
    /*      -(i) inputFrame : 80 samples (16 bits PCM)                           */
    /*      -(o) bitStream : The 15 parameters for a frame on 80 bits            */
    /*           on 80 bits (5 16bits words) for voice frame, 4 on 2 byte for    */
    /*           noise frame, 0 for untransmitted frames                         */
    /*      -(o) bitStreamLength : actual length of output, may be 0, 2 or 10    */
    /*           if VAD/DTX is enabled                                           */
    /*                                                                           */
    /*****************************************************************************/
    bcg729EncoderChannelContextStruct *context = initBcg729EncoderChannel(false);
    if (!context) {
        php_error_docref(NULL, E_WARNING, "Falha ao inicializar encoder.");
        RETURN_FALSE;
    }

    array_init(return_value);
    size_t offset = 0;
    while (offset + 160 <= input_len) {
        const int16_t *inputFrame = (const int16_t *) (input + offset);
        uint8_t bitStream[10] = {0};
        uint8_t bitStreamLength = 0;

        bcg729Encoder(
            context,
            inputFrame,
            bitStream,
            &bitStreamLength
        );


        zval frame_result;
        array_init(&frame_result);
        add_assoc_stringl(&frame_result, "output", (char *) bitStream, bitStreamLength);
        add_assoc_long(&frame_result, "length", bitStreamLength);
        add_next_index_zval(return_value, &frame_result);

        offset += 160;
    }
    closeBcg729EncoderChannel(context);
    // return value
    RETURN_ZVAL(return_value, 1, 0);
    // return_value = NULL;
}

// -------------------- REGISTRO DAS FUNÇÕES --------------------

const zend_function_entry bcg729_functions[] = {
    PHP_FE(bcg729DecodeStream, arginfo_bcg729DecodeStream)
    PHP_FE(bcg729EncodeStream, arginfo_bcg729EncodeStream)
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
