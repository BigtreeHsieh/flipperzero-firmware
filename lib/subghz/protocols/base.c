#include "base.h"
#include "registry.h"

void subghz_protocol_decoder_base_set_decoder_callback(
    SubGhzProtocolDecoderBase* decoder_base,
    SubGhzProtocolDecoderBaseRxCallback callback,
    void* context) {
    decoder_base->callback = callback;
    decoder_base->context = context;
}

SubGhzProtocolStatus subghz_protocol_decoder_base_serialize(
    SubGhzProtocolDecoderBase* decoder_base,
    string_t output) {
    SubGhzProtocolStatus status = SubGhzProtocolStatusUnknown;

    const SubGhzProtocol* protocol = decoder_base->protocol;
    if(protocol && protocol->decoder && protocol->decoder->serialize) {
        protocol->decoder->serialize(decoder_base, output);
        status = SubGhzProtocolStatusOk;
    }

    return status;
}