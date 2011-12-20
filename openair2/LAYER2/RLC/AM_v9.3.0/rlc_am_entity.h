/*******************************************************************************

Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2010 Eurecom

This program is free software; you can redistribute it and/or modify it
under the terms and conditions of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

The full GNU General Public License is included in this distribution in
the file called "COPYING".

Contact Information
Openair Admin: openair_admin@eurecom.fr
Openair Tech : openair_tech@eurecom.fr
Forums       : http://forums.eurecom.fsr/openairinterface
Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/
/*! \file rlc_am_entity.h
* \brief This file defines the RLC AM variables stored in a struct called rlc_am_entity_t.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note The rlc_am_entity_t structure store protocol variables, statistic variables, allocation variables, buffers and other miscellaneous variables.
* \bug
* \warning
*/
/** @defgroup _rlc_am_internal_impl_ RLC AM Layer Internal Reference Implementation
* @ingroup _rlc_am_impl_
* @{
*/

#    ifndef __RLC_AM_ENTITY_H__
#        define __RLC_AM_ENTITY_H__
//-----------------------------------------------------------------------------
#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "rlc_primitives.h"
#        include "rlc_def_lte.h"
#        include "rlc_am_structs.h"
#        include "rlc_am_constants.h"
//-----------------------------------------------------------------------------
/*! \struct  rlc_am_entity_t
* \brief Structure containing a RLC AM instance protocol variables, statistic variables, allocation variables, buffers and other miscellaneous variables.
*/
typedef struct rlc_am_entity {
  module_id_t     module_id;                          /*!< \brief Virtualization index for this protocol instance, means handset or eNB index. */
  u16_t           rb_id;                              /*!< \brief Radio bearer identifier, for statistics and trace purpose. */
  boolean_t       is_data_plane;                      /*!< \brief To know if the RLC belongs to a data radio bearer or a signalling radio bearer, for statistics and trace purpose. */

  signed int      sdu_buffer_occupancy;               /*!< \brief Number of bytes of unsegmented SDUs. */
  signed int      retransmission_buffer_occupancy;    /*!< \brief Number of bytes of PDUs in retransmission buffer waiting for a ACK. */
  signed int      status_buffer_occupancy;            /*!< \brief Number of bytes of control PDUs waiting for transmission. */

  //---------------------------------------------------------------------
  // TX BUFFERS
  //---------------------------------------------------------------------
  mem_block_t*                 input_sdus_alloc;      /*!< \brief Allocated memory for the input SDU buffer (for SDUs coming from upper layers). */
  rlc_am_tx_sdu_management_t   *input_sdus;           /*!< \brief Input SDU buffer (for SDUs coming from upper layers). */
  signed int      nb_sdu;                             /*!< \brief Total number of valid rlc_am_tx_sdu_management_t in input_sdus[]. */
  signed int      nb_sdu_no_segmented;                /*!< \brief Total number of SDUs not segmented and partially segmented. */
  signed int      next_sdu_index;                     /*!< \brief Next SDU index in input_sdus array where for a new incoming SDU. */
  signed int      current_sdu_index;                  /*!< \brief Current SDU index in input_sdus array to be segmented. */


  mem_block_t*                    pdu_retrans_buffer_alloc;  /*!< \brief Allocated memory for the retransmission buffer. */
  rlc_am_tx_data_pdu_management_t *pdu_retrans_buffer;       /*!< \brief Retransmission buffer. */
  signed int      retrans_num_pdus;                          /*!< \brief Number of PDUs in the retransmission buffer. */
  signed int      retrans_num_bytes;                         /*!< \brief Number of bytes in the retransmission buffer. */
  signed int      retrans_num_bytes_to_retransmit;           /*!< \brief Number of bytes in the retransmission buffer to be retransmitted. */
  unsigned int    num_nack_so;                               /*!< \brief Number of segment offsets asked to be retransmitted by peer RLC entity. */
  unsigned int    num_nack_sn;                               /*!< \brief Number of segment asked to be retransmitted by peer RLC entity. */

  //---------------------------------------------------------------------
  // RX BUFFERS
  //---------------------------------------------------------------------
  list2_t         receiver_buffer;                           /*!< \brief Receiver buffer implemented with a list. */
  mem_block_t     *output_sdu_in_construction;               /*!< \brief Memory area where a complete SDU is reassemblied before being send to upper layers. */
  s32_t           output_sdu_size_to_write;                  /*!< \brief Size of the reassemblied SDU. */


  //---------------------------------------------------------------------
  // PROTOCOL VARIABLES
  //---------------------------------------------------------------------
  u8_t            protocol_state; /*!< \brief Protocol state, can be RLC_NULL_STATE, RLC_DATA_TRANSFER_READY_STATE. */
  //-----------------------------
  // TX STATE VARIABLES
  //-----------------------------
  u16_t           vt_a;         /*!< \brief Acknowledgement state variable. This state variable holds the value of the SN of the next AMD PDU for which a positive acknowledgment is to be received in-sequence, and it serves as the lower edge of the transmitting window. It is initially set to 0, and is updated whenever the AM RLC entity receives a positive acknowledgment for an AMD PDU with SN = VT(A).*/
  u16_t           vt_ms;        /*!< \brief Maximum send state variable. This state variable equals VT(A) + AM_Window_Size, and it serves as the higher edge of the transmitting window. */
  u16_t           vt_s;         /*!< \brief  Send state variable. This state variable holds the value of the SN to be assigned for the next newly generated AMD PDU. It is initially set to 0, and is updated whenever the AM RLC entity delivers an AMD PDU with SN = VT(S).*/
  u16_t           poll_sn;      /*!< \brief  Poll send state variable. This state variable holds the value of VT(S)-1 upon the most recent transmission of a RLC data PDU with the poll bit set to “1”. It is initially set to 0.*/

  //-----------------------------
  // RX STATE VARIABLES
  //-----------------------------
  u16_t           vr_r;         /*!< \brief Receive state variable. This state variable holds the value of the SN following the last in-sequence completely received AMD PDU, and it serves as the lower edge of the receiving window. It is initially set to 0, and is updated whenever the AM RLC entity receives an AMD PDU with SN = VR(R). */
  u16_t           vr_mr;        /*!< \brief Maximum acceptable receive state variable. This state variable equals VR(R) + AM_Window_Size, and it holds the value of the SN of the first AMD PDU that is beyond the receiving window and serves as the higher edge of the receiving window. */
  u16_t           vr_x;         /*!< \brief t-Reordering state variable. This state variable holds the value of the SN following the SN of the RLC data PDU which triggered t-Reordering. */
  u16_t           vr_ms;        /*!< \brief Maximum STATUS transmit state variable. This state variable holds the highest possible value of the SN which can be indicated by “ACK_SN” when a STATUS PDU needs to be constructed. It is initially set to 0. */
  u16_t           vr_h;         /*!< \brief Highest received state variable. This state variable holds the value of the SN following the SN of the RLC data PDU with the highest SN among received RLC data PDUs. It is initially set to 0. */

  //-----------------------------
  // TIMERS CONFIGURED BY RRC
  //-----------------------------
  rlc_am_timer_t  t_poll_retransmit; /*!< \brief This timer is used by the transmitting side of an AM RLC entity in order to retransmit a poll. */
  rlc_am_timer_t  t_reordering;      /*!< \brief This timer is used by the receiving side of an AM RLC entity and receiving UM RLC entity in order to detect loss of RLC PDUs at lower layer (see sub clauses 5.1.2.2 and 5.1.3.2). If t-Reordering is running, t-Reordering shall not be started additionally, i.e. only one t-Reordering per RLC entity is running at a given time. */
  rlc_am_timer_t  t_status_prohibit; /*!< \brief This timer is used by the receiving side of an AM RLC entity in order to prohibit transmission of a STATUS PDU. */

  //-----------------------------
  // COUNTERS
  //-----------------------------
  unsigned int    c_pdu_without_poll; /*!< \brief This counter is initially set to 0. It counts the number of AMD PDUs sent since the most recent poll bit was transmitted. */
  unsigned int    c_byte_without_poll;/*!< \brief This counter is initially set to 0. It counts the number of data bytes sent since the most recent poll bit was transmitted. */

  //-----------------------------
  // PARAMETERS CONFIGURED BY RRC
  //-----------------------------
  u16_t           max_retx_threshold; /*!< \brief This parameter is used by the transmitting side of each AM RLC entity to limit the number of retransmissions of an AMD PDU. */
  u16_t           poll_pdu;           /*!< \brief This parameter is used by the transmitting side of each AM RLC entity to trigger a poll for every pollPDU PDUs. */
  u16_t           poll_byte;          /*!< \brief This parameter is used by the transmitting side of each AM RLC entity to trigger a poll for every pollByte bytes. */

  //---------------------------------------------------------------------
  // STATISTICS
  //---------------------------------------------------------------------
  unsigned int stat_tx_pdcp_sdu;                      /*!< \brief Number of SDUs received from upper layers. */
  unsigned int stat_tx_pdcp_sdu_discarded;            /*!< \brief Number of SDUs received from upper layers that have been discarded. */
  unsigned int stat_tx_retransmit_pdu_unblock;        /*!< \brief Not updated. */
  unsigned int stat_tx_retransmit_pdu_by_status;      /*!< \brief Not updated. */
  unsigned int stat_tx_retransmit_pdu;                /*!< \brief Not updated. */
  unsigned int stat_tx_data_pdu;                      /*!< \brief Not updated. */
  unsigned int stat_tx_control_pdu;                   /*!< \brief Not updated. */

  unsigned int stat_rx_sdu;                           /*!< \brief Not updated. */
  unsigned int stat_rx_error_pdu;                     /*!< \brief Not updated. */
  unsigned int stat_rx_data_pdu;                      /*!< \brief Number of data PDUs received. */
  unsigned int stat_rx_data_pdu_duplicate;            /*!< \brief Number of duplicate data PDUs received. */
  unsigned int stat_rx_data_pdu_out_of_window;        /*!< \brief Number of data PDUs received out of the receive window. */
  unsigned int stat_rx_control_pdu;                   /*!< \brief Number of control PDUs received. */

  //---------------------------------------------------------------------
  // OUTPUTS
  //---------------------------------------------------------------------
  u16_t             nb_bytes_requested_by_mac;  /*!< \brief Number of bytes requested by lower layer for next transmission. */
  list_t            pdus_to_mac_layer;          /*!< \brief PDUs buffered for transmission to MAC layer. */
  list_t            control_pdu_list;           /*!< \brief Control PDUs buffered for transmission to MAC layer. */
  s16_t             first_retrans_pdu_sn;       /*!< \brief Lowest sequence number of PDU to be retransmitted. */
  list_t            segmentation_pdu_list;      /*!< \brief List of "freshly" segmented PDUs. */

  u32_t             status_requested;             /*!< \brief Status requested by peer. */
  u32_t             last_frame_status_indication; /*!< \brief The last frame number a  MAC status indication has been received by RLC. */
  //-----------------------------
  // buffer occupancy measurements sent to MAC
  //-----------------------------
  // note occupancy of other buffers is deducted from nb elements in lists
  u32_t             buffer_occupancy_retransmission_buffer;   /*!< \brief Number of PDUs. */

  //**************************************************************
  // new members
  //**************************************************************
  u8_t              allocation;                              /*!< \brief Boolean for rlc_am_entity_t struct allocation. */
  u8_t              location;                                /*!< \brief EnodeB / UE. */

} rlc_am_entity_t;
/** @} */
#    endif
