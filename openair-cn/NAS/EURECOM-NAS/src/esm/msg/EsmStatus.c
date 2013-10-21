#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "EsmStatus.h"

int decode_esm_status(esm_status_msg *esm_status, uint8_t *buffer, uint32_t len)
{
    uint32_t decoded = 0;
    int decoded_result = 0;

    // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, ESM_STATUS_MINIMUM_LENGTH, len);

    /* Decoding mandatory fields */
    if ((decoded_result = decode_esm_cause(&esm_status->esmcause, 0, buffer + decoded, len - decoded)) < 0)
        return decoded_result;
    else
        decoded += decoded_result;

    return decoded;
}

int encode_esm_status(esm_status_msg *esm_status, uint8_t *buffer, uint32_t len)
{
    int encoded = 0;
    int encode_result = 0;

    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ESM_STATUS_MINIMUM_LENGTH, len);

    if ((encode_result = encode_esm_cause(&esm_status->esmcause, 0, buffer +
         encoded, len - encoded)) < 0)        //Return in case of error
        return encode_result;
    else
        encoded += encode_result;

    return encoded;
}

