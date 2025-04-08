#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_bcg729.h"

#include <stdint.h>
#include <string.h>
#include <zend_smart_string.h>
#include "bcg729/decoder.h"

// -------------------- ARGINFO --------------------

ZEND_BEGIN_ARG_INFO(arginfo_bcg729_hello, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729FrameToUlaw, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729PayloadToPcm, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729PacketRTPToUlaw, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, packet, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frames, IS_LONG, 1) // opcional
ZEND_END_ARG_INFO()

// -------------------- FUNÇÕES AUXILIARES --------------------

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
    return ~(sign | (exponent << 4) | mantissa);
}

// -------------------- FUNÇÕES PHP --------------------

// Retorna uma string fixa de teste
ZEND_FUNCTION(bcg729_hello)
{
    RETURN_STRING("Hello from bcg729 extension!");
}

// Converte payload G.729 para PCM 16-bit (160 bytes por frame)
ZEND_FUNCTION(g729PayloadToPcm)
{
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE)
        RETURN_FALSE;

    if (input_len % 10 != 0) {
        php_error_docref(NULL, E_WARNING, "Expected G.729 payload to be a multiple of 10 bytes.");
        RETURN_FALSE;
    }

    size_t frames = input_len / 10;
    smart_string pcm_result = {0};

    bcg729DecoderChannelContextStruct *decoder = initBcg729DecoderChannel();
    if (!decoder) {
        php_error_docref(NULL, E_WARNING, "Failed to initialize decoder.");
        RETURN_FALSE;
    }

    for (size_t i = 0; i < frames; i++) {
        const uint8_t *g729_payload = (const uint8_t *)(input + i * 10);
        int16_t pcmOut[80] = {0};
        //bcg729Decoder(decoder, frame, 0, 0, 0, 0, pcmOut);
        bcg729Decoder(decoder, g729_payload, 0, pcmOut);
        /*****************************************************************************/
        /* bcg729Decoder :                                                           */
        /*    parameters:                                                            */
        /*      -(i) decoderChannelContext : the channel context data                */
        /*      -(i) bitStream : 15 parameters on 80 bits                            */
        /*      -(i) frameErased: flag: true, frame has been erased                  */
        /*      -(o) signal : a decoded frame 80 samples (16 bits PCM)               */
        /*                                                                           */
        /*****************************************************************************/
        smart_string_appendl(&pcm_result, (const char *)pcmOut, sizeof(pcmOut));
    }

    closeBcg729DecoderChannel(decoder);
    smart_string_0(&pcm_result);
    RETURN_STRINGL(pcm_result.c, pcm_result.len);
}

// Converte payload G.729 para µ-law
ZEND_FUNCTION(g729FrameToUlaw)
{
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE)
        RETURN_FALSE;

    if (input_len % 10 != 0) {
        php_error_docref(NULL, E_WARNING, "Expected G.729 payload to be multiple of 10 bytes.");
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
        //bcg729Decoder(decoder, frame, 0, 0, 0, 0, pcmOut);
        bcg729Decoder(decoder, g729_payload, 0, pcmOut);
        /*****************************************************************************/
        /* bcg729Decoder :                                                           */
        /*    parameters:                                                            */
        /*      -(i) decoderChannelContext : the channel context data                */
        /*      -(i) bitStream : 15 parameters on 80 bits                            */
        /*      -(i) frameErased: flag: true, frame has been erased                  */
        /*      -(o) signal : a decoded frame 80 samples (16 bits PCM)               */
        /*                                                                           */
        /*****************************************************************************/
        for (int j = 0; j < 80; j++)
            smart_string_appendc(&ulaw_result, linear_to_ulaw(pcmOut[j]));
    }

    closeBcg729DecoderChannel(decoder);
    smart_string_0(&ulaw_result);
    RETURN_STRINGL(ulaw_result.c, ulaw_result.len);
}

// Converte um pacote RTP G.729 em RTP µ-law, respeitando o número de frames informado
ZEND_FUNCTION(g729PacketRTPToUlaw)
{
    char *packet;
    size_t packet_len;
    zend_long frame_count = 2; // padrão (ptime = 20ms)

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &packet, &packet_len, &frame_count) == FAILURE)
        RETURN_FALSE;

    if (frame_count <= 0) {
        php_error_docref(NULL, E_WARNING, "Número de frames deve ser maior que zero.");
        RETURN_FALSE;
    }

    size_t expected_payload = frame_count * 10;
    if (packet_len < 12 + expected_payload) {
        php_error_docref(NULL, E_WARNING, "Pacote RTP menor do que o esperado (%ld frames = %zu bytes).", frame_count, expected_payload);
        RETURN_FALSE;
    }

    const uint8_t *rtp = (const uint8_t *)packet;

    // Validação RTP
    uint8_t version = (rtp[0] >> 6) & 0x03;
    if (version != 2) {
        php_error_docref(NULL, E_WARNING, "Cabeçalho RTP inválido (versão != 2).");
        RETURN_FALSE;
    }

    uint8_t payload_type = rtp[1] & 0x7F;
    if (payload_type != 18) {
        php_error_docref(NULL, E_WARNING, "Payload Type esperado: 18 (G.729). Recebido: %d", payload_type);
        RETURN_FALSE;
    }

    size_t result_len = 12 + frame_count * 80;
    char *result = emalloc(result_len);
    if (!result) {
        php_error_docref(NULL, E_ERROR, "Falha ao alocar memória.");
        RETURN_FALSE;
    }

    memcpy(result, rtp, 12); // copia header RTP

    bcg729DecoderChannelContextStruct *decoder = initBcg729DecoderChannel();
    if (!decoder) {
        efree(result);
        php_error_docref(NULL, E_WARNING, "Falha ao iniciar decoder.");
        RETURN_FALSE;
    }

    for (zend_long i = 0; i < frame_count; i++) {
        const uint8_t *frame = (const uint8_t *)(packet + 12 + i * 10);
        int16_t pcmOut[80] = {0};
        uint8_t *ulawOut = (uint8_t *)(result + 12 + i * 80);

        //bcg729Decoder(decoder, frame, 0, 0, 0, 0, pcmOut);
        bcg729Decoder(decoder, frame, 0, pcmOut);
        /*****************************************************************************/
        /* bcg729Decoder :                                                           */
        /*    parameters:                                                            */
        /*      -(i) decoderChannelContext : the channel context data                */
        /*      -(i) bitStream : 15 parameters on 80 bits                            */
        /*      -(i) frameErased: flag: true, frame has been erased                  */
        /*      -(o) signal : a decoded frame 80 samples (16 bits PCM)               */
        /*                                                                           */
        /*****************************************************************************/

        for (int j = 0; j < 80; j++)
            ulawOut[j] = linear_to_ulaw(pcmOut[j]);
    }

    closeBcg729DecoderChannel(decoder);

    // Troca Payload Type para 0 (ULAW), preservando o bit Marker
    result[1] = (result[1] & 0x80) | 0x00;

    RETURN_STRINGL(result, result_len);
}

// -------------------- FUNÇÕES EXPORTADAS --------------------

const zend_function_entry bcg729_functions[] = {
    PHP_FE(bcg729_hello, arginfo_bcg729_hello)
    PHP_FE(g729FrameToUlaw, arginfo_g729FrameToUlaw)
    PHP_FE(g729PayloadToPcm, arginfo_g729PayloadToPcm)
    PHP_FE(g729PacketRTPToUlaw, arginfo_g729PacketRTPToUlaw)
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
