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

ZEND_BEGIN_ARG_INFO(arginfo_bcg729_hello, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729FrameToUlaw, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frames, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729PayloadToPcm, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frames, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729PacketRTPToUlaw, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, packet, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frames, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ulawPacketToG729, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, packet, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frames, IS_LONG, 1)
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

ZEND_FUNCTION(bcg729_hello)
{
    RETURN_STRING("Hello from bcg729 extension!");
}

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
        bcg729Decoder(decoder, g729_payload, 0, pcmOut);
        smart_string_appendl(&pcm_result, (const char *)pcmOut, sizeof(pcmOut));
    }

    closeBcg729DecoderChannel(decoder);
    smart_string_0(&pcm_result);
    RETURN_STRINGL(pcm_result.c, pcm_result.len);
}

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
        bcg729Decoder(decoder, g729_payload, 0, pcmOut);
        for (int j = 0; j < 80; j++)
            smart_string_appendc(&ulaw_result, linear_to_ulaw(pcmOut[j]));
    }

    closeBcg729DecoderChannel(decoder);
    smart_string_0(&ulaw_result);
    RETURN_STRINGL(ulaw_result.c, ulaw_result.len);
}

ZEND_FUNCTION(g729PacketRTPToUlaw)
{
    char *packet;
    size_t packet_len;
    zend_long frame_count = 2;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &packet, &packet_len, &frame_count) == FAILURE)
        RETURN_FALSE;

    if (frame_count <= 0) {
        php_error_docref(NULL, E_WARNING, "Número de frames deve ser maior que zero.");
        RETURN_FALSE;
    }

    size_t expected_payload = frame_count * 10;
    if (packet_len < 12 + expected_payload) {
        php_error_docref(NULL, E_WARNING, "Pacote RTP menor que o esperado.");
        RETURN_FALSE;
    }

    const uint8_t *rtp = (const uint8_t *)packet;

    uint8_t version = (rtp[0] >> 6) & 0x03;
    if (version != 2) {
        php_error_docref(NULL, E_WARNING, "Versão RTP inválida.");
        RETURN_FALSE;
    }

    size_t result_len = 12 + frame_count * 80;
    char *result = emalloc(result_len);
    if (!result) {
        php_error_docref(NULL, E_ERROR, "Erro de memória.");
        RETURN_FALSE;
    }

    memcpy(result, rtp, 12);

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
        bcg729Decoder(decoder, frame, 0, pcmOut);
        for (int j = 0; j < 80; j++)
            ulawOut[j] = linear_to_ulaw(pcmOut[j]);
    }

    closeBcg729DecoderChannel(decoder);

    result[1] = (result[1] & 0x80) | 0x00; // PT = 0 (ULAW)

    RETURN_STRINGL(result, result_len);
}

ZEND_FUNCTION(ulawPacketToG729)
{
    char *packet;
    size_t packet_len;
    zend_long frame_count = 2;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &packet, &packet_len, &frame_count) == FAILURE)
        RETURN_FALSE;

    if (frame_count <= 0) {
        php_error_docref(NULL, E_WARNING, "Número de frames deve ser maior que zero.");
        RETURN_FALSE;
    }

    size_t expected_payload = frame_count * 80;
    if (packet_len < 12 + expected_payload) {
        php_error_docref(NULL, E_WARNING, "Payload ULAW inválido.");
        RETURN_FALSE;
    }

    const uint8_t *rtp = (const uint8_t *)packet;
    const uint8_t *ulaw_payload = (const uint8_t *)(packet + 12);

    size_t result_len = 12 + frame_count * 10;
    char *result = emalloc(result_len);
    if (!result) {
        php_error_docref(NULL, E_ERROR, "Erro ao alocar memória.");
        RETURN_FALSE;
    }

    memcpy(result, rtp, 12);

    bcg729EncoderChannelContextStruct *encoder = initBcg729EncoderChannel();
    if (!encoder) {
        efree(result);
        php_error_docref(NULL, E_WARNING, "Falha ao iniciar encoder.");
        RETURN_FALSE;
    }

    for (zend_long i = 0; i < frame_count; i++) {
        const uint8_t *ulaw_frame = ulaw_payload + i * 80;
        int16_t pcm[80];
        for (int j = 0; j < 80; j++) {
            uint8_t ulaw = ~ulaw_frame[j];
            int sign = ulaw & 0x80;
            int exponent = (ulaw >> 4) & 0x07;
            int mantissa = ulaw & 0x0F;
            int sample = (((mantissa << 3) + 0x84) << exponent) - 0x84;
            pcm[j] = sign ? -sample : sample;
        }

        uint8_t *g729_out = (uint8_t *)(result + 12 + i * 10);
        bcg729Encoder(encoder, pcm, g729_out);
    }

    closeBcg729EncoderChannel(encoder);

    result[1] = (result[1] & 0x80) | 0x12; // PT = 18 (G.729)

    RETURN_STRINGL(result, result_len);
}

// -------------------- REGISTRO DAS FUNÇÕES --------------------

const zend_function_entry bcg729_functions[] = {
    PHP_FE(bcg729_hello, arginfo_bcg729_hello)
    PHP_FE(g729FrameToUlaw, arginfo_g729FrameToUlaw)
    PHP_FE(g729PayloadToPcm, arginfo_g729PayloadToPcm)
    PHP_FE(g729PacketRTPToUlaw, arginfo_g729PacketRTPToUlaw)
    PHP_FE(ulawPacketToG729, arginfo_ulawPacketToG729)
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
