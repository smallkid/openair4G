/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

*******************************************************************************/

/*
                                 platform_types.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#ifndef __PLATFORM_TYPES_H__
#    define __PLATFORM_TYPES_H__

#ifdef USER_MODE
#include <stdint.h>
#endif

#if defined(ENABLE_ITTI)
#include "itti_types.h"
#endif
//-----------------------------------------------------------------------------
// GENERIC TYPES
//-----------------------------------------------------------------------------
typedef signed char        boolean_t;

#if !defined(TRUE)
#define TRUE               (boolean_t)0x01
#endif

#if !defined(FALSE)
#define FALSE              (boolean_t)0x00
#endif

#define BOOL_NOT(b) (b^TRUE)

//-----------------------------------------------------------------------------
// GENERIC ACCESS STRATUM TYPES
//-----------------------------------------------------------------------------
typedef int32_t               sdu_size_t;
typedef uint32_t              frame_t;
typedef int32_t               sframe_t;
typedef uint32_t              sub_frame_t;
typedef uint8_t               module_id_t;
typedef uint16_t              ue_id_t;
typedef int16_t               smodule_id_t;
typedef uint16_t              rb_id_t;
typedef uint16_t              srb_id_t;

typedef boolean_t             MBMS_flag_t;
#define  MBMS_FLAG_NO         FALSE
#define  MBMS_FLAG_YES        TRUE

typedef boolean_t             eNB_flag_t;
#define  ENB_FLAG_NO          FALSE
#define  ENB_FLAG_YES         TRUE

typedef boolean_t             srb_flag_t;
#define  SRB_FLAG_NO          FALSE
#define  SRB_FLAG_YES         TRUE

typedef enum link_direction_e {
  UNKNOWN_DIR          = 0,
  DIR_UPLINK           = 1,
  DIR_DOWNLINK         = 2
} link_direction_t;

typedef enum rb_type_e {
  UNKNOWN_RADIO_BEARER        = 0,
  SIGNALLING_RADIO_BEARER     = 1,
  RADIO_ACCESS_BEARER         = 2
} rb_type_t;

//-----------------------------------------------------------------------------
// PHY TYPES
//-----------------------------------------------------------------------------
typedef uint8_t            crc8_t;
typedef uint16_t           crc16_t;
typedef uint32_t           crc32_t;
typedef unsigned int       crc_t;

//-----------------------------------------------------------------------------
// MAC TYPES
//-----------------------------------------------------------------------------
typedef sdu_size_t         tbs_size_t;
typedef sdu_size_t         tb_size_t;
typedef unsigned int       logical_chan_id_t;
typedef unsigned int       num_tb_t;
typedef uint8_t            mac_enb_index_t;

//-----------------------------------------------------------------------------
// RLC TYPES
//-----------------------------------------------------------------------------
typedef unsigned int       mui_t;
typedef unsigned int       confirm_t;
typedef unsigned int       rlc_tx_status_t;
typedef int16_t            rlc_sn_t;
typedef uint16_t           rlc_usn_t;
typedef int32_t            rlc_buffer_occupancy_t;
typedef signed int         rlc_op_status_t;

#define  SDU_CONFIRM_NO          FALSE
#define  SDU_CONFIRM_YES         TRUE
//-----------------------------------------------------------------------------
// PDCP TYPES
//-----------------------------------------------------------------------------
typedef uint16_t           pdcp_sn_t;
typedef uint32_t           pdcp_hfn_t;
typedef int16_t            pdcp_hfn_offset_t;

typedef enum pdcp_transmission_mode_e {
  PDCP_TRANSMISSION_MODE_UNKNOWN     = 0,
  PDCP_TRANSMISSION_MODE_CONTROL     = 1,
  PDCP_TRANSMISSION_MODE_DATA        = 2,
  PDCP_TRANSMISSION_MODE_TRANSPARENT = 3
} pdcp_transmission_mode_t;
//-----------------------------------------------------------------------------
// IP DRIVER / PDCP TYPES
//-----------------------------------------------------------------------------
typedef uint16_t           tcp_udp_port_t;
typedef enum  ip_traffic_type_e {
  TRAFFIC_IPVX_TYPE_UNKNOWN    =  0,
  TRAFFIC_IPV6_TYPE_UNICAST    =  1,
  TRAFFIC_IPV6_TYPE_MULTICAST  =  2,
  TRAFFIC_IPV6_TYPE_UNKNOWN    =  3,
  TRAFFIC_IPV4_TYPE_UNICAST    =  5,
  TRAFFIC_IPV4_TYPE_MULTICAST  =  6,
  TRAFFIC_IPV4_TYPE_BROADCAST  =  7,
  TRAFFIC_IPV4_TYPE_UNKNOWN    =  8
} ip_traffic_type_t;

//-----------------------------------------------------------------------------
// RRC TYPES
//-----------------------------------------------------------------------------
typedef uint32_t           mbms_session_id_t;
typedef uint16_t           mbms_service_id_t;
typedef uint16_t           rnti_t;
typedef uint8_t            rrc_enb_index_t;

#if ! defined(NOT_A_RNTI)
#define NOT_A_RNTI (rnti_t)0
#endif
#if ! defined(M_RNTI)
#define M_RNTI     (rnti_t)0xFFFD
#endif
#if ! defined(P_RNTI)
#define P_RNTI     (rnti_t)0xFFFE
#endif
#if ! defined(SI_RNTI)
#define SI_RNTI    (rnti_t)0xFFFF
#endif
typedef enum config_action_e {
  CONFIG_ACTION_NULL              = 0,
  CONFIG_ACTION_ADD               = 1,
  CONFIG_ACTION_REMOVE            = 2,
  CONFIG_ACTION_MODIFY            = 3,
  CONFIG_ACTION_SET_SECURITY_MODE = 4,
  CONFIG_ACTION_MBMS_ADD          = 10,
  CONFIG_ACTION_MBMS_MODIFY       = 11
} config_action_t;

//-----------------------------------------------------------------------------
// GTPV1U TYPES
//-----------------------------------------------------------------------------
typedef uint32_t           teid_t; // tunnel endpoint identifier
typedef uint8_t            ebi_t;  // eps bearer id



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// may be ITTI not enabled, but type instance is useful also for OTG,
#if !defined(instance_t)
typedef uint16_t instance_t;
#endif
typedef struct protocol_ctxt_s {
  module_id_t module_id;     /*!< \brief  Virtualized module identifier      */
  eNB_flag_t  enb_flag;      /*!< \brief  Flag to indicate eNB (1) or UE (0) */
  instance_t  instance;      /*!< \brief  ITTI or OTG module identifier      */
  rnti_t      rnti;
  frame_t     frame;         /*!< \brief  LTE frame number.*/
  sub_frame_t subframe;      /*!< \brief  LTE sub frame number.*/
} protocol_ctxt_t;
// warning time hardcoded
#define PROTOCOL_CTXT_TIME_MILLI_SECONDS(CtXt_h) ((CtXt_h)->frame*10+(CtXt_h)->subframe)

#define UE_MODULE_ID_TO_INSTANCE( mODULE_iD ) mODULE_iD + NB_eNB_INST
#define ENB_MODULE_ID_TO_INSTANCE( mODULE_iD ) mODULE_iD
#define UE_INSTANCE_TO_MODULE_ID( iNSTANCE ) iNSTANCE - NB_eNB_INST
#define ENB_INSTANCE_TO_MODULE_ID( iNSTANCE )iNSTANCE


#define MODULE_ID_TO_INSTANCE(mODULE_iD, iNSTANCE, eNB_fLAG) \
    if(eNB_fLAG == ENB_FLAG_YES) \
        iNSTANCE = ENB_MODULE_ID_TO_INSTANCE(mODULE_iD); \
    else \
        iNSTANCE = UE_MODULE_ID_TO_INSTANCE(mODULE_iD)

#define INSTANCE_TO_MODULE_ID(iNSTANCE, mODULE_iD, eNB_fLAG) \
    if(eNB_fLAG == ENB_FLAG_YES) \
        mODULE_iD = ENB_INSTANCE_TO_MODULE_ID(iNSTANCE); \
    else \
        mODULE_iD = UE_INSTANCE_TO_MODULE_ID(iNSTANCE)

#define PROTOCOL_CTXT_COMPUTE_MODULE_ID(CtXt_h) \
    INSTANCE_TO_MODULE_ID( (CtXt_h)->instance , (CtXt_h)->module_id , (CtXt_h)->enb_flag )


#define PROTOCOL_CTXT_COMPUTE_INSTANCE(CtXt_h) \
    MODULE_ID_TO_INSTANCE( (CtXt_h)->module_id , (CtXt_h)->instance , (CtXt_h)->enb_flag )


#define PROTOCOL_CTXT_SET_BY_MODULE_ID(Ctxt_Pp, mODULE_iD, eNB_fLAG, rNTI, fRAME, sUBfRAME) \
    (Ctxt_Pp)->module_id = mODULE_iD; \
    (Ctxt_Pp)->enb_flag  = eNB_fLAG; \
    (Ctxt_Pp)->rnti      = rNTI; \
    (Ctxt_Pp)->frame     = fRAME; \
    (Ctxt_Pp)->subframe  = sUBfRAME; \
    PROTOCOL_CTXT_COMPUTE_INSTANCE(Ctxt_Pp)

#define PROTOCOL_CTXT_SET_BY_INSTANCE(Ctxt_Pp, iNSTANCE, eNB_fLAG, rNTI, fRAME, sUBfRAME) \
    (Ctxt_Pp)->instance  = iNSTANCE; \
    (Ctxt_Pp)->enb_flag  = eNB_fLAG; \
    (Ctxt_Pp)->rnti      = rNTI; \
    (Ctxt_Pp)->frame     = fRAME; \
    (Ctxt_Pp)->subframe  = sUBfRAME; \
    PROTOCOL_CTXT_COMPUTE_MODULE_ID(Ctxt_Pp)

#define PROTOCOL_CTXT_FMT "[FRAME %05u][%s][MOD %02u][RNTI %"PRIx16"]"
#define PROTOCOL_CTXT_ARGS(CTXT_Pp) \
    (CTXT_Pp)->frame, \
    ((CTXT_Pp)->enb_flag == ENB_FLAG_YES) ? "eNB":" UE", \
    (CTXT_Pp)->module_id, \
    (CTXT_Pp)->rnti

#ifdef OAI_EMU
#define CHECK_CTXT_ARGS(CTXT_Pp) \
    if ((CTXT_Pp)->enb_flag) {\
        AssertFatal (((CTXT_Pp)->module_id >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),\
                     "eNB module id is too low (%u/%d/%d)!\n",\
                     (CTXT_Pp)->module_id,\
                     oai_emulation.info.first_enb_local,\
                     oai_emulation.info.nb_enb_local);\
        AssertFatal (((CTXT_Pp)->module_id < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),\
                     "eNB module id is too high (%u/%d)!\n",\
                     (CTXT_Pp)->module_id,\
                     oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);\
    } else {\
        AssertFatal ((CTXT_Pp)->module_id  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),\
                     "UE module id is too high (%u/%d)!\n",\
                     (CTXT_Pp)->module_id,\
                     oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);\
        AssertFatal ((CTXT_Pp)->module_id  >= oai_emulation.info.first_ue_local,\
                     "UE module id is too low (%u/%d)!\n",\
                     (CTXT_Pp)->module_id,\
                     oai_emulation.info.first_ue_local);\
    }
#else
#define CHECK_CTXT_ARGS(CTXT_Pp)
#endif
#endif
