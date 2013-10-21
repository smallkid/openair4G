#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ProtocolDiscriminator.h"
#include "SecurityHeaderType.h"
#include "MessageType.h"
#include "PagingIdentity.h"
#include "Cli.h"
#include "SsCode.h"
#include "LcsIndicator.h"
#include "LcsClientIdentity.h"

#ifndef CS_SERVICE_NOTIFICATION_H_
#define CS_SERVICE_NOTIFICATION_H_

/* Minimum length macro. Formed by minimum length of each mandatory field */
#define CS_SERVICE_NOTIFICATION_MINIMUM_LENGTH ( \
    PAGING_IDENTITY_MINIMUM_LENGTH )

/* Maximum length macro. Formed by maximum length of each field */
#define CS_SERVICE_NOTIFICATION_MAXIMUM_LENGTH ( \
    PAGING_IDENTITY_MAXIMUM_LENGTH + \
    CLI_MAXIMUM_LENGTH + \
    SS_CODE_MAXIMUM_LENGTH + \
    LCS_INDICATOR_MAXIMUM_LENGTH + \
    LCS_CLIENT_IDENTITY_MAXIMUM_LENGTH )

/* If an optional value is present and should be encoded, the corresponding
 * Bit mask should be set to 1.
 */
# define CS_SERVICE_NOTIFICATION_CLI_PRESENT                 (1<<0)
# define CS_SERVICE_NOTIFICATION_SS_CODE_PRESENT             (1<<1)
# define CS_SERVICE_NOTIFICATION_LCS_INDICATOR_PRESENT       (1<<2)
# define CS_SERVICE_NOTIFICATION_LCS_CLIENT_IDENTITY_PRESENT (1<<3)

typedef enum cs_service_notification_iei_tag {
    CS_SERVICE_NOTIFICATION_CLI_IEI                  = 0x60, /* 0x60 = 96 */
    CS_SERVICE_NOTIFICATION_SS_CODE_IEI              = 0x61, /* 0x61 = 97 */
    CS_SERVICE_NOTIFICATION_LCS_INDICATOR_IEI        = 0x62, /* 0x62 = 98 */
    CS_SERVICE_NOTIFICATION_LCS_CLIENT_IDENTITY_IEI  = 0x63, /* 0x63 = 99 */
} cs_service_notification_iei;

/*
 * Message name: CS service notification
 * Description: This message is sent by the network when a paging request with CS call indicator was received via SGs for a UE, and a NAS signalling connection is already established for the UE. See table 8.2.9.1.
 * Significance: dual
 * Direction: network to UE
 */

typedef struct cs_service_notification_msg_tag {
    /* Mandatory fields */
    ProtocolDiscriminator                protocoldiscriminator:4;
    SecurityHeaderType                   securityheadertype:4;
    MessageType                          messagetype;
    PagingIdentity                       pagingidentity;
    /* Optional fields */
    uint32_t                             presencemask;
    Cli                                  cli;
    SsCode                               sscode;
    LcsIndicator                         lcsindicator;
    LcsClientIdentity                    lcsclientidentity;
} cs_service_notification_msg;

int decode_cs_service_notification(cs_service_notification_msg *csservicenotification, uint8_t *buffer, uint32_t len);

int encode_cs_service_notification(cs_service_notification_msg *csservicenotification, uint8_t *buffer, uint32_t len);

#endif /* ! defined(CS_SERVICE_NOTIFICATION_H_) */

