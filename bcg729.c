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

// -------------------- OBJETO --------------------
static zend_object *bcg729_create(zend_class_entry *ce) {
    bcg729Channel *obj = zend_object_alloc(sizeof(bcg729Channel), ce);
    obj->decoder = initBcg729DecoderChannel();
    obj->encoder = initBcg729EncoderChannel(false);

    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    obj->std.handlers = &bcg729_handlers;

    return &obj->std;
}

static void bcg729_free(zend_object *object) {
    bcg729Channel *obj = (bcg729Channel *) ((char *) object - XtOffsetOf(bcg729Channel, std));
    if (obj->decoder) closeBcg729DecoderChannel(obj->decoder);
    if (obj->encoder) closeBcg729EncoderChannel(obj->encoder);
    zend_object_std_dtor(&obj->std);
}

// -------------------- MÉTODOS --------------------
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
    RETURN_STRINGL(result.c, result.len);
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
    RETURN_STRINGL(result.c, result.len);
}

ZEND_METHOD(bcg729Channel, info) {
    array_init(return_value);
    add_assoc_bool(return_value, "decoder_initialized", Z_BCG729_CHANNEL_P(getThis())->decoder != NULL);
    add_assoc_bool(return_value, "encoder_initialized", Z_BCG729_CHANNEL_P(getThis())->encoder != NULL);
}

// -------------------- ARGINFO --------------------
ZEND_BEGIN_ARG_INFO_EX(arginfo_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_codec_io, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

// -------------------- REGISTRO --------------------
static const zend_function_entry bcg729_methods[] = {
    ZEND_ME(bcg729Channel, __construct, arginfo_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(bcg729Channel, decode,      arginfo_codec_io, ZEND_ACC_PUBLIC)
    ZEND_ME(bcg729Channel, encode,      arginfo_codec_io, ZEND_ACC_PUBLIC)
    ZEND_ME(bcg729Channel, info,        arginfo_void,     ZEND_ACC_PUBLIC)
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

zend_module_entry bcg729_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_BCG729_EXTNAME,
    NULL,
    PHP_MINIT(bcg729),
    NULL, NULL, NULL, NULL,
    PHP_BCG729_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BCG729
ZEND_GET_MODULE(bcg729)
#endif
