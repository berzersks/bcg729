#include "php.h"
#include "php_bcg729.h"

#include <stdint.h>
#include <string.h>
#include "bcg729/decoder.h"
#include "bcg729/encoder.h"
#include <zend_smart_string.h>


#define Z_BCG729_CHANNEL_P(zv)  ((bcg729Channel *)((char *)(Z_OBJ_P(zv)) - XtOffsetOf(bcg729Channel, std)))

typedef struct {
    bcg729DecoderChannelContextStruct *decoder;
    bcg729EncoderChannelContextStruct *encoder;
    zend_object std;
} bcg729Channel;

static zend_class_entry *bcg729_ce;
static zend_object_handlers bcg729_handlers;

static zend_object *bcg729_create(zend_class_entry *ce) {
    bcg729Channel *obj = zend_object_alloc(sizeof(bcg729Channel), ce);
    obj->decoder = initBcg729DecoderChannel();
    obj->encoder = initBcg729EncoderChannel(0);

    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    obj->std.handlers = &bcg729_handlers;

    return &obj->std;
}

static void bcg729_free(zend_object *object) {
    bcg729Channel *obj = (bcg729Channel *) ((char *) object - XtOffsetOf(bcg729Channel, std));
    if (obj->decoder) {
        closeBcg729DecoderChannel(obj->decoder);
        obj->decoder = NULL;
    }
    if (obj->encoder) {
        closeBcg729EncoderChannel(obj->encoder);
        obj->encoder = NULL;
    }
    zend_object_std_dtor(&obj->std);
}


static const int16_t alaw_to_linear[256] = {
    -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
    -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
    -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
    -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
    -22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944,
    -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136,
    -11008, -10496, -12032, -11520, -8960, -8448, -9984, -9472,
    -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568,
    -344, -328, -376, -360, -280, -264, -312, -296,
    -472, -456, -504, -488, -408, -392, -440, -424,
    -88, -72, -120, -104, -24, -8, -56, -40,
    -216, -200, -248, -232, -152, -136, -184, -168,
    -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
    -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
    -688, -656, -752, -720, -560, -528, -624, -592,
    -944, -912, -1008, -976, -816, -784, -880, -848,
    5504, 5248, 6016, 5760, 4480, 4224, 4992, 4736,
    7552, 7296, 8064, 7808, 6528, 6272, 7040, 6784,
    2752, 2624, 3008, 2880, 2240, 2112, 2496, 2368,
    3776, 3648, 4032, 3904, 3264, 3136, 3520, 3392,
    22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
    30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
    11008, 10496, 12032, 11520, 8960, 8448, 9984, 9472,
    15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
    344, 328, 376, 360, 280, 264, 312, 296,
    472, 456, 504, 488, 408, 392, 440, 424,
    88, 72, 120, 104, 24, 8, 56, 40,
    216, 200, 248, 232, 152, 136, 184, 168,
    1376, 1312, 1504, 1440, 1120, 1056, 1248, 1184,
    1888, 1824, 2016, 1952, 1632, 1568, 1760, 1696,
    688, 656, 752, 720, 560, 528, 624, 592,
    944, 912, 1008, 976, 816, 784, 880, 848
};

static const int16_t ulaw_to_linear[256] = {
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
    -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
    -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
    -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316,
    -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
    -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
    -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
    -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
    -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
    -1372, -1308, -1244, -1180, -1116, -1052, -988, -924,
    -876, -844, -812, -780, -748, -716, -684, -652,
    -620, -588, -556, -524, -492, -460, -428, -396,
    -372, -356, -340, -324, -308, -292, -276, -260,
    -244, -228, -212, -196, -180, -164, -148, -132,
    -120, -112, -104, -96, -88, -80, -72, -64,
    -56, -48, -40, -32, -24, -16, -8, 0,
    32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
    23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
    15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
    11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316,
    7932, 7676, 7420, 7164, 6908, 6652, 6396, 6140,
    5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092,
    3900, 3772, 3644, 3516, 3388, 3260, 3132, 3004,
    2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980,
    1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436,
    1372, 1308, 1244, 1180, 1116, 1052, 988, 924,
    876, 844, 812, 780, 748, 716, 684, 652,
    620, 588, 556, 524, 492, 460, 428, 396,
    372, 356, 340, 324, 308, 292, 276, 260,
    244, 228, 212, 196, 180, 164, 148, 132,
    120, 112, 104, 96, 88, 80, 72, 64,
    56, 48, 40, 32, 24, 16, 8, 0
};

/**
 * Converte dados de áudio no formato A-law (PCMA) para PCM linear
 *
 * @param string $input String contendo dados codificados em A-law
 * @return string Dados convertidos para PCM linear (16-bit)
 */
ZEND_FUNCTION(decodePcmaToPcm) {
    zend_string *input;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(input) == 0) {
        RETURN_EMPTY_STRING();
    }

    smart_string result = {0};
    const unsigned char *data = (const unsigned char *)ZSTR_VAL(input);
    size_t samples = ZSTR_LEN(input);

    smart_string_alloc(&result, samples * 2, 0);

    for (size_t i = 0; i < samples; i++) {
        int16_t sample = alaw_to_linear[data[i]];
        smart_string_appendl(&result, (char *)&sample, 2);
    }

    smart_string_0(&result);
    RETVAL_STRINGL(result.c, result.len);
    smart_string_free(&result);
}


/**
 * Converte PCM little-endian para big-endian (network order)
 *
 * @param string $input Dados PCM 16-bit (little-endian)
 * @return string Dados PCM 16-bit (big-endian)
 */
ZEND_FUNCTION(pcmLeToBe)
{
    zend_string *input;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(input) < 2) {
        RETURN_EMPTY_STRING();
    }

    size_t len = ZSTR_LEN(input);
    smart_string result = {0};
    smart_string_alloc(&result, len, 0);

    const unsigned char *src = (const unsigned char *)ZSTR_VAL(input);
    for (size_t i = 0; i < len; i += 2) {
        unsigned char be[2];
        be[0] = src[i + 1];
        be[1] = src[i];
        smart_string_appendl(&result, (char *)be, 2);
    }

    smart_string_0(&result);
    RETVAL_STRINGL(result.c, result.len);
    smart_string_free(&result);
}



/**
 * Converte dados de áudio no formato μ-law (PCMU) para PCM linear
 *
 * @param string $input String contendo dados codificados em μ-law
 * @return string Dados convertidos para PCM linear (16-bit)
 */
ZEND_FUNCTION(decodePcmuToPcm) {
    zend_string *input;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(input) == 0) {
        RETURN_EMPTY_STRING();
    }

    smart_string result = {0};
    const unsigned char *data = (const unsigned char *)ZSTR_VAL(input);
    size_t samples = ZSTR_LEN(input);

    smart_string_alloc(&result, samples * 2, 0);

    for (size_t i = 0; i < samples; i++) {
        int16_t sample = ulaw_to_linear[data[i]];
        smart_string_appendl(&result, (char *)&sample, 2);
    }

    smart_string_0(&result);
    RETVAL_STRINGL(result.c, result.len);
    smart_string_free(&result);
}







static int searchSegment(int val, const int16_t *seg_end, int seg_count) {
    for (int i = 0; i < seg_count; i++) {
        if (val <= seg_end[i]) return i;
    }
    return seg_count;
}

static int linear2alaw(int pcm_val) {
    static const int16_t seg_end[8] = {0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};
    int mask;
    int seg;
    int aval;

    pcm_val >>= 3;

    if (pcm_val >= 0) {
        mask = 0xD5;
    } else {
        mask = 0x55;
        pcm_val = -pcm_val - 1;
    }

    seg = searchSegment(pcm_val, seg_end, 8);
    if (seg >= 8) return (0x7F ^ mask);

    aval = (seg << 4);
    if (seg < 2) {
        aval |= (pcm_val >> 1) & 0x0F;
    } else {
        aval |= (pcm_val >> seg) & 0x0F;
    }

    return aval ^ mask;
}

static int linear2ulaw(int pcm_val) {
    static const int16_t seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};
    int mask;
    int seg;
    int uval;

    if (pcm_val < 0) {
        pcm_val = 0x84 - pcm_val;
        mask = 0x7F;
    } else {
        pcm_val += 0x84;
        mask = 0xFF;
    }

    seg = searchSegment(pcm_val, seg_end, 8);
    if (seg >= 8) return (0x7F ^ mask);

    uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0x0F);
    return uval ^ mask;
}








ZEND_METHOD(bcg729Channel, __construct) {}



ZEND_METHOD(bcg729Channel, decode) {
    zend_string *input;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(input) % 10 != 0) {
        RETURN_FALSE;
    }

    bcg729Channel *self = Z_BCG729_CHANNEL_P(getThis());
    size_t frames = ZSTR_LEN(input) / 10;
    smart_string result = {0};

    for (size_t i = 0; i < frames; i++) {
        int16_t pcmOut[80] = {0};
        const uint8_t *frame = (const uint8_t *) ZSTR_VAL(input) + (i * 10);
        bcg729Decoder(self->decoder, frame, 10, 0, 0, 0, pcmOut);
        smart_string_appendl(&result, (char *)pcmOut, sizeof(pcmOut));
    }

    smart_string_0(&result);
    RETVAL_STRINGL(result.c, result.len);
    smart_string_free(&result);
}

ZEND_METHOD(bcg729Channel, encode) {
    zend_string *input;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(input) % 160 != 0) {
        RETURN_FALSE;
    }

    bcg729Channel *self = Z_BCG729_CHANNEL_P(getThis());
    size_t frames = ZSTR_LEN(input) / 160;
    smart_string result = {0};

    for (size_t i = 0; i < frames; i++) {
        const int16_t *pcmIn = (const int16_t *) (ZSTR_VAL(input) + i * 160);
        uint8_t g729[10];
        uint8_t len = 0;
        bcg729Encoder(self->encoder, pcmIn, g729, &len);
        smart_string_appendl(&result, (char *)g729, len);
    }

    smart_string_0(&result);
    RETVAL_STRINGL(result.c, result.len);
    smart_string_free(&result);
}

ZEND_METHOD(bcg729Channel, info) {
    array_init(return_value);
    add_assoc_bool(return_value, "decoder_initialized", Z_BCG729_CHANNEL_P(getThis())->decoder != NULL);
    add_assoc_bool(return_value, "encoder_initialized", Z_BCG729_CHANNEL_P(getThis())->encoder != NULL);
}

ZEND_METHOD(bcg729Channel, close) {
    bcg729Channel *self = Z_BCG729_CHANNEL_P(getThis());

    if (self->decoder) {
        closeBcg729DecoderChannel(self->decoder);
        self->decoder = NULL;
    }

    if (self->encoder) {
        closeBcg729EncoderChannel(self->encoder);
        self->encoder = NULL;
    }

    gc_collect_cycles();

    RETURN_TRUE;
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_codec_io, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry bcg729_methods[] = {
    ZEND_ME(bcg729Channel, __construct, arginfo_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(bcg729Channel, decode,      arginfo_codec_io, ZEND_ACC_PUBLIC)
    ZEND_ME(bcg729Channel, encode,      arginfo_codec_io, ZEND_ACC_PUBLIC)
    ZEND_ME(bcg729Channel, info,        arginfo_void,     ZEND_ACC_PUBLIC)
    ZEND_ME(bcg729Channel, close,       arginfo_void,     ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

PHP_MINIT_FUNCTION(bcg729) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "bcg729Channel", bcg729_methods);
    bcg729_ce = zend_register_internal_class(&ce);
    bcg729_ce->create_object = bcg729_create;

    memcpy(&bcg729_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    bcg729_handlers.offset = XtOffsetOf(bcg729Channel, std);
    bcg729_handlers.free_obj = bcg729_free;

    return SUCCESS;
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_decode_law, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_encode_law, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()



static const zend_function_entry bcg729_functions[] = {
    ZEND_FE(decodePcmaToPcm, arginfo_decode_law)
    ZEND_FE(decodePcmuToPcm, arginfo_decode_law)
    ZEND_FE(pcmLeToBe, arginfo_decode_law)
    ZEND_FE_END
};

zend_module_entry bcg729_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_BCG729_EXTNAME,
    bcg729_functions,
    PHP_MINIT(bcg729),
    NULL, NULL, NULL, NULL,
    PHP_BCG729_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BCG729
ZEND_GET_MODULE(bcg729)
#endif
