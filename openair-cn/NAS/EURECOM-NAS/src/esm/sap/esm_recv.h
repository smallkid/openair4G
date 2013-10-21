/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		esm_recv.h

Version		0.1

Date		2013/02/06

Product		NAS stack

Subsystem	EPS Session Management

Author		Frederic Maurel

Description	Defines functions executed at the ESM Service Access
		Point upon receiving EPS Session Management messages
		from the EPS Mobility Management sublayer.

*****************************************************************************/
#ifndef __ESM_RECV_H__
#define __ESM_RECV_H__

#include "EsmStatus.h"

#ifdef NAS_UE
#include "PdnConnectivityReject.h"
#include "PdnDisconnectReject.h"
#include "BearerResourceAllocationReject.h"
#include "BearerResourceModificationReject.h"

#include "ActivateDefaultEpsBearerContextRequest.h"
#include "ActivateDedicatedEpsBearerContextRequest.h"
#include "ModifyEpsBearerContextRequest.h"
#include "DeactivateEpsBearerContextRequest.h"

#include "EsmInformationRequest.h"
#endif

#ifdef NAS_MME
#include "PdnConnectivityRequest.h"
#include "PdnDisconnectRequest.h"
#include "BearerResourceAllocationRequest.h"
#include "BearerResourceModificationRequest.h"

#include "ActivateDefaultEpsBearerContextAccept.h"
#include "ActivateDefaultEpsBearerContextReject.h"
#include "ActivateDedicatedEpsBearerContextAccept.h"
#include "ActivateDedicatedEpsBearerContextReject.h"
#include "ModifyEpsBearerContextAccept.h"
#include "ModifyEpsBearerContextReject.h"
#include "DeactivateEpsBearerContextAccept.h"

#include "EsmInformationResponse.h"
#endif

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * Functions executed by both the UE and the MME upon receiving ESM messages
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
int esm_recv_status(int pti, int ebi, const esm_status_msg* msg);
#endif
#ifdef NAS_MME
int esm_recv_status(unsigned int ueid, int pti, int ebi, const esm_status_msg* msg);
#endif

/*
 * --------------------------------------------------------------------------
 * Functions executed by the UE upon receiving ESM message from the network
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/*
 * Transaction related messages
 * ----------------------------
 */
int esm_recv_pdn_connectivity_reject(int pti, int ebi, const pdn_connectivity_reject_msg* msg);

int esm_recv_pdn_disconnect_reject(int pti, int ebi, const pdn_disconnect_reject_msg* msg);

/*
 * Messages related to EPS bearer contexts
 * ---------------------------------------
 */
int esm_recv_activate_default_eps_bearer_context_request(int pti, int ebi, const activate_default_eps_bearer_context_request_msg* msg);

int esm_recv_activate_dedicated_eps_bearer_context_request(int pti, int ebi, const activate_dedicated_eps_bearer_context_request_msg* msg);

int esm_recv_deactivate_eps_bearer_context_request(int pti, int ebi, const deactivate_eps_bearer_context_request_msg* msg);
#endif

/*
 * --------------------------------------------------------------------------
 * Functions executed by the MME upon receiving ESM message from the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Transaction related messages
 * ----------------------------
 */
int esm_recv_pdn_connectivity_request(unsigned int ueid, int pti, int ebi, const pdn_connectivity_request_msg* msg, unsigned int* new_ebi, void* data);

int esm_recv_pdn_disconnect_request(unsigned int ueid, int pti, int ebi, const pdn_disconnect_request_msg* msg, unsigned int* linked_ebi);

/*
 * Messages related to EPS bearer contexts
 * ---------------------------------------
 */
int esm_recv_activate_default_eps_bearer_context_accept(unsigned int ueid, int pti, int ebi, const activate_default_eps_bearer_context_accept_msg* msg);

int esm_recv_activate_default_eps_bearer_context_reject(unsigned int ueid, int pti, int ebi, const activate_default_eps_bearer_context_reject_msg* msg);

int esm_recv_activate_dedicated_eps_bearer_context_accept(unsigned int ueid, int pti, int ebi, const activate_dedicated_eps_bearer_context_accept_msg* msg);

int esm_recv_activate_dedicated_eps_bearer_context_reject(unsigned int ueid, int pti, int ebi, const activate_dedicated_eps_bearer_context_reject_msg* msg);

int esm_recv_deactivate_eps_bearer_context_accept(unsigned int ueid, int pti, int ebi, const deactivate_eps_bearer_context_accept_msg* msg);
#endif

#endif /* __ESM_RECV_H__*/
