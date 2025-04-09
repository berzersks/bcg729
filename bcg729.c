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
