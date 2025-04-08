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

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729PayloadToPcm, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_g729PacketRTPToUlaw, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, packet, IS_STRING, 0)
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

/**
 * g729PayloadToPcm(string $input): string
 *
 * Decodifica um payload G.729 (múltiplos de 10 bytes) para dados PCM 16-bit.
 * Cada quadro G.729 (10 bytes) é decodificado para 80 amostras PCM (160 bytes).
 *
 * @param string $input  Payload G.729 (cada frame com 10 bytes)
 * @return string        Dados PCM (160 bytes por frame) ou FALSE em caso de erro.
 */
ZEND_FUNCTION(g729PayloadToPcm)
{
    char *input;
    size_t input_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &input, &input_len) == FAILURE) {
        RETURN_FALSE;
    }

    /* Verifica se o tamanho da entrada é um múltiplo de 10 bytes */
    if (input_len % 10 != 0) {
        php_error_docref(NULL, E_WARNING, "Expected G.729 payload to be a multiple of 10 bytes per frame.");
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

        /* Decodifica o frame G.729 para obter 80 amostras PCM */
        bcg729Decoder(decoder, g729_payload, 0, 0, 0, 0, pcmOut);

        /* Acrescenta 160 bytes (80 amostras de 2 bytes cada) ao resultado */
        smart_string_appendl(&pcm_result, (const char *)pcmOut, sizeof(pcmOut));
    }

    closeBcg729DecoderChannel(decoder);
    smart_string_0(&pcm_result);

    RETURN_STRINGL(pcm_result.c, pcm_result.len);
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

ZEND_FUNCTION(g729PacketRTPToUlaw)
{
    char *packet;
    size_t packet_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &packet, &packet_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (packet_len < 12 + 10) {
        php_error_docref(NULL, E_WARNING, "Pacote RTP muito pequeno (esperado no mínimo 12 + 10 bytes).");
        RETURN_FALSE;
    }

    const uint8_t *rtp = (const uint8_t *)packet;

    // Validação do header RTP
    uint8_t version = (rtp[0] >> 6) & 0x03;
    if (version != 2) {
        php_error_docref(NULL, E_WARNING, "Cabeçalho RTP inválido (versão != 2).");
        RETURN_FALSE;
    }

    uint8_t payload_type = rtp[1] & 0x7F;
    if (payload_type != 18) {
        php_error_docref(NULL, E_WARNING, "Payload type não é G.729 (esperado 18). Recebido: %d", payload_type);
        RETURN_FALSE;
    }

    size_t payload_len = packet_len - 12;
    if (payload_len != 10) {
        php_error_docref(NULL, E_WARNING, "Payload G.729 com tamanho inválido (esperado 10 bytes). Recebido: %zu", payload_len);
        RETURN_FALSE;
    }

    const uint8_t *g729_payload = (const uint8_t *)(packet + 12);
    int16_t pcmOut[80] = {0};
    uint8_t ulaw_payload[80];

    bcg729DecoderChannelContextStruct *decoder = initBcg729DecoderChannel();
    if (!decoder) {
        php_error_docref(NULL, E_WARNING, "Falha ao iniciar decoder.");
        RETURN_FALSE;
    }

    bcg729Decoder(decoder, g729_payload, 0, 0, 0, 0, pcmOut);
    closeBcg729DecoderChannel(decoder);

    for (int i = 0; i < 80; i++) {
        ulaw_payload[i] = linear_to_ulaw(pcmOut[i]);
    }

    // Monta novo pacote RTP com header original + novo payload
    size_t result_len = 12 + 80;
    char *result = emalloc(result_len);
    memcpy(result, rtp, 12);
    memcpy(result + 12, ulaw_payload, 80);

    // Corrige o Payload Type para 0 (ULAW), mantendo o Marker bit original
    result[1] = (result[1] & 0x80) | 0x00;

    RETURN_STRINGL(result, result_len);
}


// Lista de funções expostas ao PHP
const zend_function_entry bcg729_functions[] = {
    PHP_FE(bcg729_hello, arginfo_bcg729_hello)
    PHP_FE(g729FrameToUlaw, arginfo_g729FrameToUlaw)
    PHP_FE(g729PayloadToPcm, arginfo_g729PayloadToPcm)
    PHP_FE(g729PacketRTPToUlaw, arginfo_g729PacketRTPToUlaw)
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
