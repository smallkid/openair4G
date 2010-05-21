#include <string.h>
#include <math.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif
#include "SCHED/defs.h"
#include "SCHED/vars.h"

#include "PHY/LTE_TRANSPORT/mcs_tbs_tools.h"
#define SECONDARY_SYSTEM
#ifdef SECONDARY_SYSTEM
#include "decl_secsys.h"
#endif
//#define SKIP_RF_CHAIN
//#define CHANNEL_FROM_FILE
//#define FLAT_CHANNEL
//#define SKIP_RF_RX
#define BW 10.0
#define Td 1.0
#define N_TRIALS 1

//#define DEBUG_PHY


DCI0_5MHz_TDD0_t          UL_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

#define UL_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,24)
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fff


int main(int argc, char **argv) {

  int i,j,l,aa,sector,i_max,l_max,aa_max;
  double sigma2, sigma2_dB=0;
  mod_sym_t **txdataF;
#ifdef SECONDARY_SYSTEM
  mod_sym_t **txdataF_ext[2];
#endif
#ifdef IFFT_FPGA
  int **txdataF2;
#ifdef SECONDARY_SYSTEM
  int **txdataF2_secsys;
#endif
#endif
   int **txdata,**rxdata;
#ifdef SECONDARY_SYSTEM
   int **txdata_ext[2], **rxdata_ext[2];
#endif
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.5; //0.0000005;
  int channel_length;
  struct complex **ch;
  unsigned char pbch_pdu[6];
  int sync_pos, sync_pos_slot;
  FILE *rx_frame_file;
  int result;
  int freq_offset;
  int subframe_offset;
  char fname[40], vname[40];
  int trial, n_errors=0;
  unsigned int nb_rb = 25;
  unsigned int first_rb = 0;
  unsigned int eNb_id = 0;
#ifdef SECONDARY_SYSTEM
  unsigned int eNb_id_secsys = 0;
#endif
  unsigned int slot_offset, slot_offset_time;
  int n_frames;

  int slot,last_slot, next_slot;

  double rx_gain_lin;

  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB, tx_pwr, rx_pwr;
#ifdef SECONDARY_SYSTEM
  enum UNIT_ID {
    PeNb,    /// PeNb
    SeNb,    /// SeNb
    P_UE,    /// P_UE
    S_UE    /// S_UE
  };
  enum CH_ID {
    PeSe,    /// PeNb <--> SeNb
    PeSu,    /// PeNb <--> S_UE
    SePu,    /// SeNb <--> P_UE
    PuSu,    /// P_UE <--> S_UE
    PePu,    /// PeNb <--> P_UE
    SeSu     /// SeNb <--> S_UE
  };
  double path_loss_ar_dB[6], path_loss_ar[6];
  struct complex **ch_ar[6];
  double tx_pwr_secsys, rx_pwr_secsys, SIR;
  int SIRdB = 0;
  double **s_re_ext[2],**s_im_ext[2],**r_re_ext[2],**r_im_ext[2];
  double **r_re_crossLink[6],**r_im_crossLink[6]; //all entries with i==j will be zero, denoting the channel to itself.
  FILE *channel_file;
  char has_channel;
  char channel_output[] = "channel_output.txt";
#endif
  int rx_pwr2;

  unsigned char first_call = 1, first_call_secsys_DL = 1, first_call_secsys_UL = 1;

  char stats_buffer[4096];
  int len;

#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif

  if (argc>1)
    sigma2_dB = atoi(argv[1]);

  if (argc>2)
    n_frames = atoi(argv[2]);
  else
    n_frames = 2;

  channel_length = (int) 11+2*BW*Td;
#ifdef SECONDARY_SYSTEM
  double ch_tmp[2*channel_length*4*6]; // #ofElementsInComplex*channel_length*(nbRx*nbTx)*#ofChannels
#endif

  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

#ifndef SECONDARY_SYSTEM
  PHY_VARS_eNB *PHY_vars_eNb[1]; // 1 eNBs
  PHY_vars_eNb[0] = malloc(sizeof(PHY_VARS_eNB));
  PHY_VARS_UE *PHY_vars_UE[1]; // 1 UEs
  PHY_vars_UE[0] = malloc(sizeof(PHY_VARS_UE));
#else //SECONDARY_SYSTEM
  PHY_VARS_eNB *PHY_vars_eNb[3]; // 3 eNBs
  PHY_vars_eNb[0] = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNb[1] = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNb[2] = malloc(sizeof(PHY_VARS_eNB)); //virtual eNb
  PHY_VARS_UE *PHY_vars_UE[3]; // 3 UEs
  PHY_vars_UE[0] = malloc(sizeof(PHY_VARS_UE));
  PHY_vars_UE[1] = malloc(sizeof(PHY_VARS_UE));
  PHY_vars_UE[2] = malloc(sizeof(PHY_VARS_UE)); //virtual UE
  //  PHY_config_secsys = malloc(sizeof(PHY_CONFIG));
  //  mac_xfaec_secsys = malloc(sizeof(MAC_xface));
#endif

  //lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_frame_parms = &(PHY_vars_eNb[0]->lte_frame_parms);
  
  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->N_RB_UL            = 25;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = 2;
  lte_frame_parms->nb_antennas_rx     = 2;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = 6;
  lte_frame_parms->Csrs = 2;
  lte_frame_parms->Bsrs = 0;
  lte_frame_parms->kTC = 0;
  lte_frame_parms->n_RRC = 0;

  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(NB_ANTENNAS_TX);
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  lte_gold(lte_frame_parms);
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();

  msg("[PHY_vars_UE] = %p",PHY_vars_UE);

  lte_sync_time_init(lte_frame_parms);

  //use same frame parameters for UE as for eNb
  PHY_vars_UE[0]->lte_frame_parms = *lte_frame_parms;
  PHY_vars_UE[0]->lte_frame_parms.nb_antennas_rx = 1;
  PHY_vars_UE[0]->lte_frame_parms.nb_antennas_tx = 1; 
#ifdef SECONDARY_SYSTEM
  //use same frame parameters for secondary system
  PHY_vars_eNb[1]->lte_frame_parms = *lte_frame_parms;
  PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_tx = 1;
  PHY_vars_eNb[1]->nb_virtual_tx = 1;  // # of virtual antennas tx
  //use same frame parameters for UE as for eNb
  PHY_vars_UE[1]->lte_frame_parms = *lte_frame_parms;

  /// VIRTUAL UNITS, used for comparative measurements
  //next user used as a copy of the primary receiver, with only interference for comparative measurements
  PHY_vars_UE[2]->lte_frame_parms = *lte_frame_parms;
  PHY_vars_UE[2]->lte_frame_parms.nb_antennas_rx = 1;
  PHY_vars_UE[2]->lte_frame_parms.nb_antennas_tx = 1; 
  PHY_vars_eNb[2]->lte_frame_parms = *lte_frame_parms;
#endif
 
  phy_init_lte_ue(lte_frame_parms,
		  &PHY_vars_UE[0]->lte_ue_common_vars,
		  PHY_vars_UE[0]->lte_ue_dlsch_vars,
		  PHY_vars_UE[0]->lte_ue_dlsch_vars_cntl,
		  PHY_vars_UE[0]->lte_ue_pbch_vars,
		  PHY_vars_UE[0]->lte_ue_pdcch_vars);
  PHY_vars_UE[0]->is_secondary_ue = 0;

  PHY_vars_eNb[0]->is_secondary_eNb = 0;
  phy_init_lte_eNB(lte_frame_parms,
		   &PHY_vars_eNb[0]->lte_eNB_common_vars,
		   PHY_vars_eNb[0]->lte_eNB_ulsch_vars,
		   PHY_vars_eNb[0]->is_secondary_eNb,
		   PHY_vars_eNb[1]);
  PHY_vars_eNb[0]->is_init_sync = 0; // not used for primary eNb
#ifdef SECONDARY_SYSTEM
  phy_init_lte_ue(lte_frame_parms,
		  &PHY_vars_UE[1]->lte_ue_common_vars,
		  PHY_vars_UE[1]->lte_ue_dlsch_vars,
		  PHY_vars_UE[1]->lte_ue_dlsch_vars_cntl,
		  PHY_vars_UE[1]->lte_ue_pbch_vars,
		  PHY_vars_UE[1]->lte_ue_pdcch_vars);
  PHY_vars_UE[1]->is_secondary_ue = 1;
  PHY_vars_UE[1]->has_valid_precoder = 0;

  phy_init_lte_ue(lte_frame_parms,
		  &PHY_vars_UE[2]->lte_ue_common_vars,
		  PHY_vars_UE[2]->lte_ue_dlsch_vars,
		  PHY_vars_UE[2]->lte_ue_dlsch_vars_cntl,
		  PHY_vars_UE[2]->lte_ue_pbch_vars,
		  PHY_vars_UE[2]->lte_ue_pdcch_vars);
  PHY_vars_UE[2]->is_secondary_ue = 0;


  PHY_vars_eNb[1]->is_secondary_eNb = 1;
  phy_init_lte_eNB(lte_frame_parms,
		   &PHY_vars_eNb[1]->lte_eNB_common_vars,
		   PHY_vars_eNb[1]->lte_eNB_ulsch_vars,
		   PHY_vars_eNb[1]->is_secondary_eNb,
		   PHY_vars_eNb[1]);
  PHY_vars_eNb[1]->is_init_sync = 0;
  PHY_vars_eNb[1]->has_valid_precoder = 0;
  //Virtual eNb
  PHY_vars_eNb[2]->is_secondary_eNb = 0;
  phy_init_lte_eNB(lte_frame_parms,
		   &PHY_vars_eNb[2]->lte_eNB_common_vars,
		   PHY_vars_eNb[2]->lte_eNB_ulsch_vars,
		   PHY_vars_eNb[2]->is_secondary_eNb,
		   PHY_vars_eNb[2]);
  PHY_vars_eNb[2]->is_init_sync = 0;
  PHY_vars_eNb[2]->has_valid_precoder = 0;
#endif

#ifndef SECONDARY_SYSTEM
  aa_max = 1; //number of eNBs
  l_max = 1; //number of UEs
#else //SECONDARY_SYSTEM
  aa_max = 3; //number of eNBs
  l_max = 3; //number of UEs
#endif

//loop over eNBs
for (aa=0;aa<aa_max;aa++) {
  //loop over UEs
  for (l=0;l<l_max;l++) {
    //loop over transport channels per DLSCH
    for (i=0;i<2;i++) {
      PHY_vars_eNb[aa]->dlsch_eNb[l][i] = new_eNb_dlsch(1,8);
      if (!PHY_vars_eNb[aa]->dlsch_eNb[l][i]) {
	msg("Can't get eNb ulsch structures\n");
	exit(-1);
      } else {
	msg("PHY_vars_eNb[%d]->dlsch_eNb[%d][%d] = %p\n",aa,l,i,PHY_vars_eNb[aa]->dlsch_eNb[l][i]);
      }
      PHY_vars_UE[l]->dlsch_ue[aa][i] = new_ue_dlsch(1,8);
      if (!PHY_vars_UE[l]->dlsch_ue[aa][i]) {
	msg("Can't get ue ulsch structures\n");
	exit(-1);
      } else {
	msg("PHY_vars_UE [%d]->dlsch_ue [%d][%d] = %p\n",l,aa,i,PHY_vars_UE[l]->dlsch_ue[aa][i]);
      }
    }
  }
}

//loop over eNBs
for (aa=0;aa<aa_max;aa++) {
  //loop over UEs
  for (l=0;l<l_max;l++) {
    PHY_vars_eNb[aa]->ulsch_eNb[l] = new_eNb_ulsch(3);
    if (!PHY_vars_eNb[aa]->ulsch_eNb[l]) {
      msg("Can't get eNb ulsch structures\n");
      exit(-1);
    } else {
      msg("PHY_vars_eNb[%d]->ulsch_eNb[%d] = %p\n",aa,l,PHY_vars_eNb[aa]->ulsch_eNb[l]);
    }
    PHY_vars_UE[l]->ulsch_ue[aa] = new_ue_ulsch(3);
    if (!PHY_vars_UE[l]->ulsch_ue[aa]) {
      msg("Can't get ue ulsch structures\n");
      exit(-1);
    } else {
      msg("PHY_vars_UE [%d]->ulsch_ue [%d] = %p\n",l,aa,PHY_vars_UE[l]->ulsch_ue[aa]);
    }
  }
}

  PHY_vars_eNb[0]->dlsch_eNb_cntl = new_eNb_dlsch(1,1);
  PHY_vars_UE[0]->dlsch_ue_cntl  = new_ue_dlsch(1,1);
#ifdef SECONDARY_SYSTEM
  PHY_vars_eNb[1]->dlsch_eNb_cntl = new_eNb_dlsch(1,1);
  PHY_vars_eNb[2]->dlsch_eNb_cntl = new_eNb_dlsch(1,1);
  PHY_vars_UE[1]->dlsch_ue_cntl  = new_ue_dlsch(1,1);
  PHY_vars_UE[2]->dlsch_ue_cntl  = new_ue_dlsch(1,1);

  txdataF_rep_tmp = (int **)malloc(2*(PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_tx+PHY_vars_eNb[1]->nb_virtual_tx)*sizeof(int*));
  for (aa=0; aa<(PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_tx+PHY_vars_eNb[1]->nb_virtual_tx); aa++) {
    txdataF_rep_tmp[aa] = (int *)malloc16(2*sizeof(int)*(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size)); // repeated format (hence the '2*')
    if (txdataF_rep_tmp[aa]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] txdataF_rep_tmp[%d] allocated at %p\n",aa,txdataF_rep_tmp[aa]);
#endif
    bzero(txdataF_rep_tmp[aa],2*(sizeof(int))*(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size));
    }
    else {
      msg("[openair][LTE_PHY][INIT] txdataF_rep_tmp[%d] not allocated\n",aa);
      return(-1);
    }
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] txdataF_rep_tmp[%d] = %p, length = %d\n",aa,txdataF_rep_tmp[aa],2*(sizeof(int))*(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size));
#endif
  }
#endif
 

  unsigned char m_mcs,m_I_tbs;
              //SE = 1
  m_I_tbs = SE2I_TBS(.5, lte_frame_parms->N_RB_DL, lte_frame_parms->num_dlsch_symbols);
  m_mcs = I_TBS2I_MCS(m_I_tbs);

  // init DCI structures for testing
  UL_alloc_pdu.type    = 0;
  UL_alloc_pdu.hopping = 0;
  UL_alloc_pdu.rballoc = UL_RB_ALLOC;
  UL_alloc_pdu.mcs     = 1;
  UL_alloc_pdu.ndi     = 1;
  UL_alloc_pdu.TPC     = 0;
  UL_alloc_pdu.cqi_req = 1;

  CCCH_alloc_pdu.type               = 0;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.rv       = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 1;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = 4; // m_mcs;
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2.tpmi             = 0;


#ifdef IFFT_FPGA
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);

  bzero(txdata[0],FRAME_LENGTH_BYTES);
  bzero(txdata[1],FRAME_LENGTH_BYTES);

  rxdata    = (int **)malloc16(2*sizeof(int*));
  rxdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  rxdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);

  bzero(rxdata[0],FRAME_LENGTH_BYTES);
  bzero(rxdata[1],FRAME_LENGTH_BYTES);
  txdataF2    = (int **)malloc16(2*sizeof(int*));
  txdataF2[0] = (int *)malloc16(FRAME_LENGTH_BYTES_NO_PREFIX);
  txdataF2[1] = (int *)malloc16(FRAME_LENGTH_BYTES_NO_PREFIX);

  bzero(txdataF2[0],FRAME_LENGTH_BYTES_NO_PREFIX);
  bzero(txdataF2[1],FRAME_LENGTH_BYTES_NO_PREFIX);
#endif
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
#ifdef SECONDARY_SYSTEM
  for (i=0; i<2; i++) {
    s_re_ext[i] = malloc(2*sizeof(double*));
    s_im_ext[i] = malloc(2*sizeof(double*));
    r_re_ext[i] = malloc(2*sizeof(double*));
    r_im_ext[i] = malloc(2*sizeof(double*));
  }
  for (l=0; l<6; l++) {
    r_re_crossLink[l] = malloc(2*sizeof(double*));
    r_im_crossLink[l] = malloc(2*sizeof(double*));
  }
#endif
  
  for (i=0;i<2;i++) { //loop over antennas

    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));

#ifdef SECONDARY_SYSTEM
    for (j=0; j<2; j++) {
      s_re_ext[j][i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      bzero(s_re_ext[j][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      s_im_ext[j][i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      bzero(s_im_ext[j][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      r_re_ext[j][i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      bzero(r_re_ext[j][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      r_im_ext[j][i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      bzero(r_im_ext[j][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    }
    for (l=0; l<6; l++) { //loop over channel index
      r_re_crossLink[l][i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      bzero(r_re_crossLink[l][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      r_im_crossLink[l][i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
      bzero(r_im_crossLink[l][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    } // channel index
#endif
  }

  ch = (struct complex**) malloc(4 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));
#ifdef SECONDARY_SYSTEM
  for (l=0; l<6; l++) {
    ch_ar[l] = (struct complex**) malloc(4 * sizeof(struct complex*));
    for (i = 0; i<4; i++)
      ch_ar[l][i] = (struct complex*) malloc(channel_length * sizeof(struct complex));
  }

  randominit(0);
  set_taus_seed(0);

  channel_file = fopen(channel_output,"r");
  if (channel_file) {
    if (fscanf(channel_file,"%d",&has_channel) != EOF) {
      if (has_channel == 1) {
	for (aa=0; aa<6; aa++) { //loop over channel index
	  for (i=0; i<lte_frame_parms->nb_antennas_rx; i++) {
	    for (j=0; j<lte_frame_parms->nb_antennas_tx; j++) {
	      for (l=0; l<channel_length; l++) {
		if (fscanf(channel_file,"%lf %lf",&ch_ar[aa][i + j*2][l].r, &ch_ar[aa][i + j*2][l].i) != 2) break;
	      }
	    }
	  }
	}
      }
    }
    fclose(channel_file);
  }
 
#ifdef FLAT_CHANNEL

  struct complex ch_const[6][lte_frame_parms->nb_antennas_rx+lte_frame_parms->nb_antennas_tx];

  double realCh_const[4];
  double imagCh_const[4];

  for (i=0; i<4; i++) {
    realCh_const[i] = gaussdouble(0.0,1.0);
    imagCh_const[i] = gaussdouble(0.0,1.0);
  }

  for (aa=0; aa<6; aa++) { //loop over channel index
    for (i=0; i<lte_frame_parms->nb_antennas_rx; i++) {
      for (j=0; j<lte_frame_parms->nb_antennas_tx; j++) {
	ch_const[aa][i +j*2].r = gaussdouble(0.0,1.0);
	ch_const[aa][i +j*2].i = gaussdouble(0.0,1.0);
	if (has_channel) {
	  ch_const[aa][i +j*2].r = ch_ar[aa][i + j*2][0].r;
	  ch_const[aa][i +j*2].i = ch_ar[aa][i + j*2][0].i;
	} else {
	  msg("ch_const[%i][%i]->r = %lf \n",aa,i +j*2,ch_const[aa][i +j*2].r);
	  msg("ch_const[%i][%i]->i = %lf \n",aa,i +j*2,ch_const[aa][i +j*2].i);
	  ch_ar[aa][i + j*2][0].r = ch_const[aa][i +j*2].r;
	  ch_ar[aa][i + j*2][0].i = ch_const[aa][i +j*2].i;
	}
	for (l=1; l<channel_length; l++) {
	  ch_ar[aa][i + j*2][l].r = 0;
	  ch_ar[aa][i + j*2][l].i = 0;
	}
      }
    }
  }
  has_channel = 1;

    for (i=0; i<lte_frame_parms->nb_antennas_rx; i++) {
      for (j=0; j<lte_frame_parms->nb_antennas_tx; j++) {
	PHY_vars_eNb[1]->const_ch[i + j*2][0] = (ch_const[SePu][i + j*2].r)*64;
	PHY_vars_eNb[1]->const_ch[i + j*2][1] = (ch_const[SePu][i + j*2].i)*64;
      }
    }

    int knownData[lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1][20][2][2];//[samples][slot][antennas][real-0/imag-1]
    char knownDataFlag = 0;
    int check =0;
  channel_file = fopen("knownData.txt","r");
  
  if (channel_file) {
    if (fscanf(channel_file,"%d",&knownDataFlag) != EOF) {
      if (knownDataFlag == 1) {
	for (aa=0; aa<2; aa++) { //loop over antennas
	  for (j=0; j<20; j++) {
	    for (l=0; l<(lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1); l++) {
	      check = fscanf(channel_file,"%d %d",&knownData[l][j][aa][0], &knownData[l][j][aa][1]);
	      if (check == 2) {
	      }
	      else {
		msg("read wrong!\n");
		break;
	      }
	    }
	  }
	}
      }
    }
    fclose(channel_file);
  }

#endif //FLAT_CHANNEL

#ifndef CHANNEL_FROM_FILE
  has_channel = 0;
  first_call = 1;
  first_call_secsys_DL = 1;
  first_call_secsys_UL = 1;
#endif //CHANNEL_FROM_FILE

 
#endif //SECONDARY_SYSTEM


#ifdef SECONDARY_SYSTEM
  PHY_vars_eNb[1]->lte_frame_parms.mode1_flag = 1;
  printf("SeNb - transmission mode %d\n",PHY_vars_eNb[1]->lte_frame_parms.mode1_flag);
#endif

  openair_daq_vars.tdd = 1;
  openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;
  PHY_vars_eNb[0]->rx_total_gain_dB = 140;
#ifdef SECONDARY_SYSTEM			
  PHY_vars_eNb[1]->rx_total_gain_dB = 140;
  PHY_vars_eNb[2]->rx_total_gain_dB = 140;
#endif			

  for (mac_xface->frame=0; mac_xface->frame<n_frames; mac_xface->frame++) {

    for (slot=0 ; slot<20 ; slot++) {
      last_slot = (slot - 1)%20;
      if (last_slot <0)
	last_slot+=20;
      next_slot = (slot + 1)%20;
      
      /*-------------------------------------------------------------
	                   ALL LTE PROCESSING
      ---------------------------------------------------------------*/

      printf("\n");
      printf("Frame %d, slot %d : eNB procedures\n",mac_xface->frame,slot);
      mac_xface->is_cluster_head = 1;
      phy_procedures_eNb_lte(last_slot,next_slot,PHY_vars_eNb[0]);
#ifdef SECONDARY_SYSTEM
      printf("\n");
      printf("Frame %d, slot %d : Secondary eNB procedures\n",mac_xface->frame,slot);
      /*
      //try with known data
      if (mac_xface->frame>0 || next_slot>1 && (subframe_select_tdd(lte_frame_parms->tdd_config,last_slot>>1)==SF_DL)) {
	slot_offset = (next_slot)*(lte_frame_parms->ofdm_symbol_size)*((lte_frame_parms->Ncp==1) ? 6 : 7);
	//known data for debugging
	for (aa=0; aa<2; aa++) { //antenna
	  for (l=0; l<(lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1); l++) {
	    ((short *)(PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[0][aa]))[(slot_offset+l)<<1] = (short) knownData[l][next_slot][aa][0]; //real
	    ((short *)(PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[0][aa]))[((slot_offset+l)<<1)+1] = (short) knownData[l][next_slot][aa][1]; //imag
	  }
	}
      }
      */
      phy_procedures_eNb_lte(last_slot,next_slot,PHY_vars_eNb[1]);
      printf("\n");
      printf("Frame %d, slot %d : Virtual Primary eNB procedures\n",mac_xface->frame,slot);
      phy_procedures_eNb_lte(last_slot,next_slot,PHY_vars_eNb[2]);
#endif
      printf("\n\n");
      printf("Frame %d, slot %d : UE procedures\n",mac_xface->frame,slot);
      mac_xface->is_cluster_head = 0;      
      phy_procedures_ue_lte(last_slot,next_slot,PHY_vars_UE[0]);
#ifdef SECONDARY_SYSTEM
      printf("\n");
      printf("Frame %d, slot %d : Secondary UE procedures\n",mac_xface->frame,slot);
      phy_procedures_ue_lte(last_slot,next_slot,PHY_vars_UE[1]);
      printf("\n");
      printf("Frame %d, slot %d : Virtual Primary UE procedures\n",mac_xface->frame,slot);
      phy_procedures_ue_lte(last_slot,next_slot,PHY_vars_UE[2]);
#endif
      printf("\n");


      /*-------------------------------------------------------------
	ASSIGN POINTERS TO CORRECT BUFFERS ACCORDING TO TDD-STRUCTURE
	                  -----  TX PART  -----
                 and perform OFDM modulation ifndef IFFT_FPGA
      ---------------------------------------------------------------*/

      if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL) {
	txdataF = PHY_vars_eNb[0]->lte_eNB_common_vars.txdataF[eNb_id];
#ifdef SECONDARY_SYSTEM
	txdataF_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id_secsys];
	txdataF_ext[1] = PHY_vars_eNb[2]->lte_eNB_common_vars.txdataF[eNb_id_secsys];
#endif
#ifndef IFFT_FPGA
	txdata = PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id];
#ifdef SECONDARY_SYSTEM
	txdata_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys];
	txdata_ext[1] = PHY_vars_eNb[2]->lte_eNB_common_vars.txdata[eNb_id_secsys];
#endif
#endif
      }
      else if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_UL) {
	txdataF = PHY_vars_UE[0]->lte_ue_common_vars.txdataF;
#ifdef SECONDARY_SYSTEM
	txdataF_ext[0] = PHY_vars_UE[1]->lte_ue_common_vars.txdataF;
	txdataF_ext[1] = PHY_vars_UE[2]->lte_ue_common_vars.txdataF;
#endif
#ifndef IFFT_FPGA
	txdata = PHY_vars_UE[0]->lte_ue_common_vars.txdata;
#ifdef SECONDARY_SYSTEM
	txdata_ext[0] = PHY_vars_UE[1]->lte_ue_common_vars.txdata;
	txdata_ext[1] = PHY_vars_UE[2]->lte_ue_common_vars.txdata;
#endif
#endif
      }
      else //it must be a special subframe
	//which also means that SECONDARY system must listen, and synchronize as an UE, every x(=10) frame(s). PSS located in the 3rd symbol in this slot.
	if (next_slot%2==0) {//DL part
	  txdataF = PHY_vars_eNb[0]->lte_eNB_common_vars.txdataF[eNb_id];
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here, primary will transmit
	  txdataF_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id_secsys]; // should point to NULL (though this will now just point to a lot of zeros, since phy_procedures routine will not generate PSS).
	  txdataF_ext[1] = PHY_vars_eNb[2]->lte_eNB_common_vars.txdataF[eNb_id_secsys];
#endif
#ifndef IFFT_FPGA
	  txdata = PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id];
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here, primary will transmit
	  txdata_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys]; // should point to NULL (though this will now just point to a lot of zeros, since phy_procedures routine will not generate PSS).
	  txdata_ext[1] = PHY_vars_eNb[2]->lte_eNB_common_vars.txdata[eNb_id_secsys];
#endif
#endif
	}
	else {// UL part
	  txdataF = PHY_vars_UE[0]->lte_ue_common_vars.txdataF;
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here
	  txdataF_ext[0] = PHY_vars_UE[1]->lte_ue_common_vars.txdataF;
	  txdataF_ext[1] = PHY_vars_UE[2]->lte_ue_common_vars.txdataF;
#endif
#ifndef IFFT_FPGA
	  txdata = PHY_vars_UE[0]->lte_ue_common_vars.txdata;
#ifdef SECONDARY_SYSTEM // SEC_SYS should be in Rx-mode here
	  txdata_ext[0] = PHY_vars_UE[1]->lte_ue_common_vars.txdata;
	  txdata_ext[1] = PHY_vars_UE[2]->lte_ue_common_vars.txdata;
#endif
#endif
	}


#ifdef IFFT_FPGA

      slot_offset = (next_slot)*(lte_frame_parms->N_RB_DL*12)*((lte_frame_parms->Ncp==1) ? 6 : 7);

      //write_output("eNb_txsigF0.m","eNb_txsF0", PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][0],300*120,1,4);
      //write_output("eNb_txsigF1.m","eNb_txsF1", PHY_vars_eNb->lte_eNB_common_vars->txdataF[eNb_id][1],300*120,1,4);

      
      // do table lookup and write results to txdataF2
      for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {

	l = slot_offset;	
	for (i=0;i<NUMBER_OF_OFDM_CARRIERS*((lte_frame_parms->Ncp==1) ? 6 : 7);i++) 
	  if ((i%512>=1) && (i%512<=150))
	    txdataF2[aa][i] = ((int*)mod_table)[txdataF[aa][l++]];
	  else if (i%512>=362)
	    txdataF2[aa][i] = ((int*)mod_table)[txdataF[aa][l++]];
	  else 
	    txdataF2[aa][i] = 0;
#ifdef SECONDARY_SYSTEM
	  if ((i%512>=1) && (i%512<=150))
	    txdataF2_secsys[aa][i] = ((int*)mod_table)[txdataF_ext[0][aa][l++]];
	  else if (i%512>=362)
	    txdataF2_secsys[aa][i] = ((int*)mod_table)[txdataF_ext[0][aa][l++]];
	  else 
	    txdataF2_secsys[aa][i] = 0;
#endif
      }
      
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) 
	PHY_ofdm_mod(txdataF2[aa],        // input
		     txdata[aa],         // output
		     lte_frame_parms->log2_symbol_size,                // log2_fft_size
		     (lte_frame_parms->Ncp==1) ? 6 : 7,                 // number of symbols
		     lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		     lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     lte_frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
#ifdef SECONDARY_SYSTEM
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) 
	PHY_ofdm_mod(txdataF2_secsys[aa],        // input
		     txdata_ext[0][aa],         // output
		     lte_frame_parms->log2_symbol_size,
		     (lte_frame_parms->Ncp==1) ? 6 : 7,
		     lte_frame_parms->nb_prefix_samples
		     lte_frame_parms->twiddle_ifft,
		     lte_frame_parms->rev,
		     CYCLIC_PREFIX);
#endif //SECONDARY_SYSTEM
      
#else //IFFT_FPGA

      slot_offset = (next_slot)*(lte_frame_parms->ofdm_symbol_size)*((lte_frame_parms->Ncp==1) ? 6 : 7);
      //      printf("Copying TX buffer for slot %d (%d)\n",next_slot,slot_offset);
      slot_offset_time = (next_slot)*(lte_frame_parms->samples_per_tti>>1);

      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
	PHY_ofdm_mod(&txdataF[aa][slot_offset],        // input
		     &txdata[aa][slot_offset_time],    // output
		     lte_frame_parms->log2_symbol_size,                // log2_fft_size
		     (lte_frame_parms->Ncp==1) ? 6 : 7,                 // number of symbols
		     lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		     lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     lte_frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      } 
 
#ifdef SECONDARY_SYSTEM
      
    if (next_slot==11) {
      write_output("txdataF_a0_.m","txF_a0_",&PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id][0][next_slot*(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size)*(PHY_vars_eNb[1]->lte_frame_parms.symbols_per_tti>>1)],(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size)*(PHY_vars_eNb[1]->lte_frame_parms.symbols_per_tti>>1),1,1);
      write_output("txdataF_a1_.m","txF_a1_",&PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id][1][next_slot*(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size)*(PHY_vars_eNb[1]->lte_frame_parms.symbols_per_tti>>1)],(PHY_vars_eNb[1]->lte_frame_parms.ofdm_symbol_size)*(PHY_vars_eNb[1]->lte_frame_parms.symbols_per_tti>>1),1,1);
    }
      if (next_slot==11) {
	write_output("txdata_f0.m","txs_f0",&txdataF_ext[0][0][slot_offset],(lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1),1,1);
	write_output("txdata_f1.m","txs_f1",&txdataF_ext[0][1][slot_offset],(lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1),1,1);
      }
      
      for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
	PHY_ofdm_mod(&txdataF_ext[0][aa][slot_offset],// input
		     &txdata_ext[0][aa][slot_offset_time],// output
		     lte_frame_parms->log2_symbol_size,
		     (lte_frame_parms->Ncp==1) ? 6 : 7,
		     lte_frame_parms->nb_prefix_samples,
		     lte_frame_parms->twiddle_ifft,
		     lte_frame_parms->rev, CYCLIC_PREFIX);
      }  
      if (next_slot==11) {
	write_output("txdata_f0_known.m","txs_f0_known",&txdataF_ext[0][0][slot_offset],(lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1),1,1);
	write_output("txdata_f1_known.m","txs_f1_known",&txdataF_ext[0][1][slot_offset],(lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti>>1),1,1);
	write_output("txdata_t_known.m","txs_t_known",&txdata_ext[0][0][slot_offset_time],lte_frame_parms->samples_per_tti>>1,1,1);
      }
#endif
#endif //IFFT_FPGA

    
      /*-------------------------------------------------------------
	ASSIGN POINTERS TO CORRECT BUFFERS ACCORDING TO TDD-STRUCTURE
	                  -----  RX PART  -----
      ---------------------------------------------------------------*/
  
  if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL)
    rxdata = PHY_vars_UE[0]->lte_ue_common_vars.rxdata;
  else if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_UL)
    rxdata = PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id];
  else //it must be a special subframe
    if (next_slot%2==0) //DL part
      rxdata = PHY_vars_UE[0]->lte_ue_common_vars.rxdata;
    else // UL part
      rxdata = PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id];
 

#ifdef SECONDARY_SYSTEM

  if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL) {
    rxdata_ext[0] = PHY_vars_UE[1]->lte_ue_common_vars.rxdata;
    rxdata_ext[1] = PHY_vars_UE[2]->lte_ue_common_vars.rxdata;
  } 
  else if (subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_UL) {
    rxdata_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
    rxdata_ext[1] = PHY_vars_eNb[2]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
  }
  else {//it must be a special subframe
    //which also means that SECONDARY system must listen, and synchronize as an UE, every x(=10) frame(s) or every frame the on power up. PSS is located in the 3rd symbol in this slot.
    if (PHY_vars_eNb[1]->is_init_sync && mac_xface->frame%10>0) {
      rxdata_ext[1] = PHY_vars_UE[2]->lte_ue_common_vars.rxdata;
      if (next_slot%2==0) { //DL part
	rxdata_ext[0] = PHY_vars_UE[1]->lte_ue_common_vars.rxdata;
      }
      else {  //UL part
	rxdata_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
      }
    } 
    else { //UL part
      	rxdata_ext[0] = PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
      	rxdata_ext[1] = PHY_vars_eNb[2]->lte_eNB_common_vars.rxdata[eNb_id_secsys];
    }
  }


#endif //SECONDARY_SYSTEM

      /*-------------------------------------------------------------
	                 TRANSMISSION SIMULATION
      ---------------------------------------------------------------*/
  
#ifdef SKIP_RF_CHAIN
      // get pointer to data ready to be transmitted
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  s_re[aa][i] = (double)(((short *)txdata[aa])[(slot_offset_time+i)<<1]);
	  s_im[aa][i] = (double)(((short *)txdata[aa])[((slot_offset_time+i)<<1)+1]);
#ifdef SECONDARY_SYSTEM
	  s_re_ext[0][aa][i] = (double)(((short *)txdata_ext[0][aa])[(slot_offset_time+i)<<1]);
	  s_im_ext[0][aa][i] = (double)(((short *)txdata_ext[0][aa])[((slot_offset_time+i)<<1)+1]);
	  s_re_ext[1][aa][i] = (double)(((short *)txdata_ext[1][aa])[(slot_offset_time+i)<<1]);
	  s_im_ext[1][aa][i] = (double)(((short *)txdata_ext[1][aa])[((slot_offset_time+i)<<1)+1]);
#endif
	}
      }
#ifdef SECONDARY_SYSTEM
  SIR = pow(10,(double)(SIRdB)/10);
#endif
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  r_re[aa][i] = s_re[aa][i];
	  r_im[aa][i] = s_im[aa][i];
#ifdef SECONDARY_SYSTEM
	  r_re_ext[0][aa][i] = s_re_ext[0][aa][i] +s_re[aa][i]*sqrt(1.0/(2*SIR));
	  r_im_ext[0][aa][i] = s_im_ext[0][aa][i] +s_im[aa][i]*sqrt(1.0/(2*SIR));
	  r_re_ext[1][aa][i] = s_re_ext[1][aa][i] +s_re[aa][i]*sqrt(1.0/(2*SIR));
	  r_im_ext[1][aa][i] = s_im_ext[1][aa][i] +s_im[aa][i]*sqrt(1.0/(2*SIR));
#endif
	}
      }

  slot_offset_time = next_slot*(lte_frame_parms->samples_per_tti>>1);
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  ((short *)rxdata[aa])[((i+slot_offset_time)<<1)]   = (short)(r_re[aa][i+0]*1);
	  ((short *)rxdata[aa])[1+((i+slot_offset_time)<<1)] = (short)(r_im[aa][i+0]*1);
#ifdef SECONDARY_SYSTEM
	  ((short *)rxdata_ext[0][aa])[((i+slot_offset_time)<<1)]   = (short)(r_re_ext[0][aa][i+0]*1);
	  ((short *)rxdata_ext[0][aa])[1+((i+slot_offset_time)<<1)] = (short)(r_im_ext[0][aa][i+0]*1);
	  ((short *)rxdata_ext[1][aa])[((i+slot_offset_time)<<1)]   = (short)(r_re_ext[1][aa][i+0]*1);
	  ((short *)rxdata_ext[1][aa])[1+((i+slot_offset_time)<<1)] = (short)(r_im_ext[1][aa][i+0]*1);
#endif
	}
      }

#else //SKIP_RF_CHAIN

      /*-------------------------------------------------------------
                                   D/A
      ---------------------------------------------------------------*/

      // convert to floating point
      tx_pwr = dac_fixed_gain(s_re,
			      s_im,
			      txdata,
			      slot_offset_time,
			      2,
			      lte_frame_parms->samples_per_tti>>1,
			      14,
			      0);
      printf("tx_pwr %f dB for slot %d (subframe %d)\n",10*log10(tx_pwr),next_slot,next_slot>>1);

#ifdef SECONDARY_SYSTEM
      for (j=0; j<1; j++) {
	// convert to floating point
	tx_pwr = dac_fixed_gain(s_re_ext[j],
				s_im_ext[j],
				txdata_ext[j],
				slot_offset_time,
				2,
				lte_frame_parms->samples_per_tti>>1,
				14,
				0);
	printf("tx_pwr_secsys %f dB for slot %d (subframe %d)\n",10*log10(tx_pwr),next_slot,next_slot>>1);
      }
      if (next_slot==11) {
	write_output("txdata_t_a0.m","txs_t_a0",&txdata_ext[0][0][slot_offset_time],lte_frame_parms->samples_per_tti>>1,1,1);
	write_output("txdata_t_a1.m","txs_t_a1",&txdata_ext[0][1][slot_offset_time],lte_frame_parms->samples_per_tti>>1,1,1);
	write_output("txdata_t_real_a0.m","txs_t_r_a0",s_re_ext[0][0],lte_frame_parms->samples_per_tti>>1,1,7);
	write_output("txdata_t_real_a1.m","txs_t_r_a1",s_re_ext[0][1],lte_frame_parms->samples_per_tti>>1,1,7);
      }

#endif

     
      /*-------------------------------------------------------------
	                     CHANNEL MODEL 
                         ANTENNA(s) TO ANTENNA(s)
      ---------------------------------------------------------------*/


//      printf("channel for slot %d (subframe %d)\n",next_slot,next_slot>>1);
      multipath_channel(
#ifdef SECONDARY_SYSTEM
			ch_ar[PePu],
#else
			ch,
#endif
			s_re,s_im,r_re,r_im,
			amps,Td,BW,ricean_factor,aoa,
			lte_frame_parms->nb_antennas_tx,
			lte_frame_parms->nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call == 1) ? 1 : 0),
#ifdef SECONDARY_SYSTEM
			(has_channel) ? 1 : 
#endif			
			((next_slot>>1==1 || first_call==1) ? 0 : 1),
#ifdef SECONDARY_SYSTEM
			PePu
#else
			0
#endif			
);
      if (first_call == 1)
	first_call = 0;

      if (next_slot==5)
	write_output("channel0.m","chan0",ch[0],channel_length,1,8);


#ifdef SECONDARY_SYSTEM   
//      printf("channel for slot %d (subframe %d)\n",next_slot,next_slot>>1);
      // SeNb to S_UE
      multipath_channel(ch_ar[SeSu],s_re_ext[0],s_im_ext[0],
			r_re_ext[0],r_im_ext[0],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_tx + PHY_vars_eNb[1]->nb_virtual_tx,
			PHY_vars_UE[1]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_DL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_DL==1) ? 0 : 1),
			SeSu);
      msg("ch_ar[%i][0][0].r = %lf\n",SeSu,ch_ar[SeSu][0][0].r);

      // channel models for interference paths
      if ((subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL) || ((subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_S) && (next_slot%2==0))) { // DL
	// from PeNb to SeNb
	multipath_channel(ch_ar[PeSe],s_re,s_im,
			r_re_crossLink[PeSe],r_im_crossLink[PeSe],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_eNb[0]->lte_frame_parms.nb_antennas_tx,
			PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_DL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_DL==1) ? 0 : 1),
			PeSe);
      msg("ch_ar[%i][0][0].r = %lf\n",PeSe,ch_ar[PeSe][0][0].r);
	// from PeNb to S_UE
	multipath_channel(ch_ar[PeSu],s_re,s_im,
			r_re_crossLink[PeSu],r_im_crossLink[PeSu],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_eNb[0]->lte_frame_parms.nb_antennas_tx,
			PHY_vars_UE[1]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_DL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_DL==1) ? 0 : 1),
			PeSu);
      msg("ch_ar[%i][0][0].r = %lf\n",PeSu,ch_ar[PeSu][0][0].r);
	// from SeNb to PeNb
	multipath_channel(ch_ar[PeSe],s_re_ext[0],s_im_ext[0],
			r_re_crossLink[PeSe],r_im_crossLink[PeSe],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_tx + PHY_vars_eNb[1]->nb_virtual_tx,
			PHY_vars_eNb[0]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_DL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_DL==1) ? 0 : 1),
			PeSe);
	// from SeNb to P_UE
	multipath_channel(ch_ar[SePu],s_re_ext[0],s_im_ext[0],
			r_re_crossLink[SePu],r_im_crossLink[SePu],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_tx + PHY_vars_eNb[1]->nb_virtual_tx,
			PHY_vars_UE[0]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_DL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_DL==1) ? 0 : 1),
			SePu);
      msg("ch_ar[%i][0][0].r = %lf\n",SePu,ch_ar[SePu][0][0].r);
      if (first_call_secsys_DL == 1)
	first_call_secsys_DL = 0;
      } else { //UL
	// from P_UE to SeNb
	multipath_channel(ch_ar[SePu],s_re,s_im,
			r_re_crossLink[SePu],r_im_crossLink[SePu],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_UE[0]->lte_frame_parms.nb_antennas_tx,
			PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_UL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_UL==1) ? 0 : 1),
			SePu);
      msg("ch_ar[%i][0][0].r = %lf\n",SePu,ch_ar[SePu][0][0].r);
	// from P_UE to S_UE
	multipath_channel(ch_ar[PuSu],s_re,s_im,
			r_re_crossLink[PuSu],r_im_crossLink[PuSu],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_UE[1]->lte_frame_parms.nb_antennas_rx,
			PHY_vars_UE[0]->lte_frame_parms.nb_antennas_tx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_UL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_UL==1) ? 0 : 1),
			PuSu);
      msg("ch_ar[%i][0][0].r = %lf\n",PuSu,ch_ar[PuSu][0][0].r);
	// from S_UE to P_eNb
	multipath_channel(ch_ar[PeSu],s_re_ext[0],s_im_ext[0],
			r_re_crossLink[PeSu],r_im_crossLink[PeSu],
			amps,Td,BW,ricean_factor,aoa,
			PHY_vars_UE[1]->lte_frame_parms.nb_antennas_tx,
			PHY_vars_eNb[0]->lte_frame_parms.nb_antennas_rx,
			OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*(7-lte_frame_parms->Ncp),
			channel_length,
			0,
			1, //forgetting factor (temporal variation, block stationary)
			((first_call_secsys_UL == 1) ? 1 : 0),
			(has_channel) ? 1 : ((next_slot>>1==1 || first_call_secsys_UL==1) ? 0 : 1),
			PeSu);
      msg("ch_ar[%i][0][0].r = %lf\n",PeSu,ch_ar[PeSu][0][0].r);
      if (first_call_secsys_UL == 1)
	first_call_secsys_UL = 0;
      }

#endif //SECONDARY_SYSTEM 


#ifdef SECONDARY_SYSTEM
  SIR = pow(10,(double)(SIRdB)/10);
#endif
      path_loss_dB = -70;
      path_loss    = pow(10,path_loss_dB/10);
#ifdef SECONDARY_SYSTEM
      path_loss_ar_dB[SeSu] = -70;
      path_loss_ar[SeSu]    = pow(10,path_loss_ar_dB[SeSu]/10);
      path_loss_ar_dB[SePu] = -70;
      path_loss_ar[SePu]    = pow(10,path_loss_ar_dB[SePu]/10);
      path_loss_ar_dB[PeSe] = -70;
      path_loss_ar[PeSe]    = pow(10,path_loss_ar_dB[PeSe]/10);
      path_loss_ar_dB[PeSu] = -70;
      path_loss_ar[PeSu]    = pow(10,path_loss_ar_dB[PeSu]/10);
#endif //SECONDARY_SYSTEM
      
      for (i=0;i<(lte_frame_parms->samples_per_tti>>1);i++) {
	for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
	  r_re[aa][i]=r_re[aa][i]*sqrt(path_loss/2); 
	  r_im[aa][i]=r_im[aa][i]*sqrt(path_loss/2);
#ifdef SECONDARY_SYSTEM
	  r_re_ext[0][aa][i] = r_re_ext[0][aa][i]*sqrt(path_loss_ar[SeSu]/2); 
	  r_im_ext[0][aa][i] = r_im_ext[0][aa][i]*sqrt(path_loss_ar[SeSu]/2);
	  if ((subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_S) && (next_slot%2==0)) { // SF_S DL
	    // from PeNb to SeNb
	    r_re_ext[0][aa][i] += r_re_crossLink[PeSe][aa][i]*sqrt(path_loss_ar[PeSe]/2); 
	    r_im_ext[0][aa][i] += r_im_crossLink[PeSe][aa][i]*sqrt(path_loss_ar[PeSe]/2);
	    // from PeNb to S_UE
	    r_re_ext[0][aa][i] += r_re_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2); 
	    r_im_ext[0][aa][i] += r_im_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2);
	    // from SeNb to P_UE
	    r_re[aa][i] += r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im[aa][i] += r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    // from SeNb to virtual P_UE
	    r_re_ext[1][aa][i]  = r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im_ext[1][aa][i]  = r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	  } else if ((subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_S) && (next_slot%2==1)) { // SF_S UL
	    // from P_UE to SeNb
	    r_re_ext[0][aa][i] += r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im_ext[0][aa][i] += r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2);
	    // from S_UE to PeNb
	    r_re[aa][i] += r_re_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2); 
	    r_im[aa][i] += r_im_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2); 
	  } else if ((subframe_select_tdd(lte_frame_parms->tdd_config,next_slot>>1) == SF_DL)) { // DL
	    // from PeNb to S_UE
	    r_re_ext[0][aa][i] += r_re_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2); 
	    r_im_ext[0][aa][i] += r_im_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2);
	    // from SeNb to P_UE
	    r_re[aa][i] += r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im[aa][i] += r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    // from SeNb to virtual P_UE
	    r_re_ext[1][aa][i] = r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im_ext[1][aa][i] = r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	  } else { // UL
	    // from P_UE to SeNb
	    r_re_ext[0][aa][i] += r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im_ext[0][aa][i] += r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    // from S_UE to PeNb
	    r_re[aa][i] += r_re_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2); 
	    r_im[aa][i] += r_im_crossLink[PeSu][aa][i]*sqrt(path_loss_ar[PeSu]/2); 
	  }
	  /*
	  if (next_slot>>1==5) {
	    // from SeNb to P_UE
	    r_re[aa][i] = r_re_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	    r_im[aa][i] = r_im_crossLink[SePu][aa][i]*sqrt(path_loss_ar[SePu]/2); 
	  }
	  */
        
#endif //SECONDARY_SYSTEM
	}
      }

      if (next_slot==11) {
	write_output("rxCross_t_real.m","rxsCr_t_r",r_re_crossLink[2][0],lte_frame_parms->samples_per_tti>>1,1,7);
	write_output("rxdata_t_real.m","rxs_t_r",r_re_ext[1][0],lte_frame_parms->samples_per_tti>>1,1,7);
      }
      /*-------------------------------------------------------------
	                       RF MODELLING 
                                 RX PART
      ---------------------------------------------------------------*/

#ifndef SKIP_RF_RX
      rf_rx(r_re,
	    r_im,
	    NULL,
	    NULL,
	    0,
	    lte_frame_parms->nb_antennas_rx,
	    lte_frame_parms->samples_per_tti>>1,
	    1.0/7.68e6 * 1e9,      // sampling time (ns)
	    500,            // freq offset (Hz) (-20kHz..20kHz)
	    0.0,            // drift (Hz) NOT YET IMPLEMENTED
	    nf,             // noise_figure NOT YET IMPLEMENTED
	    (double)PHY_vars_eNb[0]->rx_total_gain_dB-72.247,            // rx_gain (dB)
	    200,            // IP3_dBm (dBm)
	    &ip,            // initial phase
	    30.0e3,         // pn_cutoff (kHz)
	    -500.0,          // pn_amp (dBc) default: 50
	    0.0,           // IQ imbalance (dB),
	    0.0);           // IQ phase imbalance (rad)

#else //SKIP_RF_RX
      N0W = pow(10.0,.1*(-174.0 - 10*log10((1.0/7.68e6 * 1e9)*1e-9)));
      rx_gain_lin = pow(10.0,.05*((double)PHY_vars_eNb[0]->rx_total_gain_dB-72.247));
      for (aa=0; aa<PHY_vars_eNb[0]->lte_frame_parms.nb_antennas_rx; aa++) {
	for (i=0; i<PHY_vars_eNb[0]->lte_frame_parms.samples_per_tti>>1; i++) {
	  r_re[aa][i] = rx_gain_lin*(r_re[aa][i] + (sqrt(.5*N0W)*gaussdouble(0.0,1.0)));
	  r_im[aa][i] = rx_gain_lin*(r_im[aa][i] + (sqrt(.5*N0W)*gaussdouble(0.0,1.0)));
	}
      }
#endif //SKIP_RF_RX

      rx_pwr = signal_energy_fp(r_re,r_im,lte_frame_parms->nb_antennas_rx,lte_frame_parms->samples_per_tti>>1,0);
 
      printf("rx_pwr (ADC in) %f dB for slot %d (subframe %d)\n",10*log10(rx_pwr),next_slot,next_slot>>1); 


#ifdef SECONDARY_SYSTEM
      for (j=0; j<2; j++) {
      // RF model
#ifndef SKIP_RF_RX
	rf_rx(r_re_ext[j],
	      r_im_ext[j],
	      NULL,
	      NULL,
	      0,
	      lte_frame_parms->nb_antennas_rx,
	      lte_frame_parms->samples_per_tti>>1,
	      1.0/7.68e6 * 1e9,      // sampling time (ns)
	      500,            // freq offset (Hz) (-20kHz..20kHz)
	      0.0,            // drift (Hz) NOT YET IMPLEMENTED
	      nf,             // noise_figure NOT YET IMPLEMENTED
	      (double)PHY_vars_eNb[j+1]->rx_total_gain_dB-72.247,            // rx_gain (dB)
	      200,            // IP3_dBm (dBm)
	      &ip,            // initial phase
	      30.0e3,         // pn_cutoff (kHz)
	      -500.0,          // pn_amp (dBc) default: 50
	      0.0,           // IQ imbalance (dB),
	      0.0);           // IQ phase imbalance (rad)
      
#else //SKIP_RF_RX
	N0W = pow(10.0,.1*(-174.0 - 10*log10((1.0/7.68e6 * 1e9)*1e-9)));
	rx_gain_lin = pow(10.0,.05*((double)PHY_vars_eNb[1]->rx_total_gain_dB-72.247));
	for (aa=0; aa<PHY_vars_eNb[1]->lte_frame_parms.nb_antennas_rx; aa++) {
	  for (i=0; i<PHY_vars_eNb[1]->lte_frame_parms.samples_per_tti>>1; i++) {
	    r_re_ext[j][aa][i] = rx_gain_lin*(r_re_ext[j][aa][i] + (sqrt(.5*N0W)*gaussdouble(0.0,1.0)));
	    r_im_ext[j][aa][i] = rx_gain_lin*(r_im_ext[j][aa][i] + (sqrt(.5*N0W)*gaussdouble(0.0,1.0)));
	  }
	}

#endif //SKIP_RF_RX

	rx_pwr = signal_energy_fp(r_re_ext[j],r_im_ext[j],lte_frame_parms->nb_antennas_rx,lte_frame_parms->samples_per_tti>>1,0);
 
	printf("rx_pwr_ext[%i] (ADC in) %f dB for slot %d (subframe %d)\n",j,10*log10(rx_pwr),next_slot,next_slot>>1); 

#endif //SECONDARY_SYSTEM
      } //loop over secondary transmission simulations

      /*-------------------------------------------------------------
	                     A/D CONVERSION
			     (QUANTIZATION)
      ---------------------------------------------------------------*/

  adc(r_re,
      r_im,
      0,
      slot_offset_time,
      rxdata,
      2,
      lte_frame_parms->samples_per_tti>>1,
      12);
  

  rx_pwr2 = signal_energy(rxdata[0]+slot_offset_time,lte_frame_parms->samples_per_tti>>1);
  
  printf("rx_pwr (ADC out) %f dB (%d) for slot %d (subframe %d)\n",10*log10((double)rx_pwr2),rx_pwr2,next_slot,next_slot>>1);  
  
#ifdef SECONDARY_SYSTEM
  for (j=0; j<2; j++) {
    adc(r_re_ext[j],
	r_im_ext[j],
	0,
	slot_offset_time,
	rxdata_ext[j],
	2,
	lte_frame_parms->samples_per_tti>>1,
	12);
    
    rx_pwr2 = signal_energy(rxdata_ext[j][0]+slot_offset_time,lte_frame_parms->samples_per_tti>>1);
    
    printf("rx_pwr_ext[%i] (ADC out) %f dB (%d) for slot %d (subframe %d)\n",j,10*log10((double)rx_pwr2),rx_pwr2,next_slot,next_slot>>1); 
  }
#endif //SECONDARY_SYSTEM
#endif //SKIP_RF_CHAIN

   if (last_slot == 11) {
     // print received signal in DL subframe 5 at primary UE. This is the subframe where there is (when debugging) only interference. Spectrum should resemble that of white noise.
     write_output("UE_rxInterference.m","UE_rxI", PHY_vars_UE[0]->lte_ue_common_vars.rxdataF[0],PHY_vars_UE[0]->lte_frame_parms.ofdm_symbol_size*PHY_vars_UE[0]->lte_frame_parms.symbols_per_tti,1,1);
   }
      if (next_slot == 19) {
    write_output("UE_rxsig0.m","UE_rxs0", PHY_vars_UE[0]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_rxsig1.m","UE_rxs1", PHY_vars_UE[0]->lte_ue_common_vars.rxdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig0.m","eNb_rxs0", PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig1.m","eNb_rxs1", PHY_vars_eNb[0]->lte_eNB_common_vars.rxdata[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
 
    write_output("UE_txsig0.m","UE_txs0", PHY_vars_UE[0]->lte_ue_common_vars.txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_txsig1.m","UE_txs1", PHY_vars_UE[0]->lte_ue_common_vars.txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig0.m","eNb_txs0", PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig1.m","eNb_txs1", PHY_vars_eNb[0]->lte_eNB_common_vars.txdata[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  }

#ifdef SECONDARY_SYSTEM
if (next_slot == 19) {
    write_output("UE_rxsig0_1.m","UE_rxs0_1", PHY_vars_UE[1]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_rxsig1_1.m","UE_rxs1_1", PHY_vars_UE[1]->lte_ue_common_vars.rxdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_rxsig0_2.m","UE_rxs0_2", PHY_vars_UE[2]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_rxsig1_2.m","UE_rxs1_2", PHY_vars_UE[2]->lte_ue_common_vars.rxdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig0_1.m","eNb_rxs0_1", PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_rxsig1_1.m","eNb_rxs1_1", PHY_vars_eNb[1]->lte_eNB_common_vars.rxdata[eNb_id_secsys][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
 
    write_output("UE_txsig0_1.m","UE_txs0_1", PHY_vars_UE[1]->lte_ue_common_vars.txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("UE_txsig1_1.m","UE_txs1_1", PHY_vars_UE[1]->lte_ue_common_vars.txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig0_1.m","eNb_txs0_1", PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    write_output("eNb_txsig1_1.m","eNb_txs1_1", PHY_vars_eNb[1]->lte_eNB_common_vars.txdata[eNb_id_secsys][1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);

    write_output("eNb_txsigF0_1.m","eNb_txsF0_1", PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id_secsys][0],lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti*10,1,1);
    write_output("eNb_txsigF1_1.m","eNb_txsF1_1", PHY_vars_eNb[1]->lte_eNB_common_vars.txdataF[eNb_id_secsys][1],lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti*10,1,1);
    write_output("eNb_txsigF0.m","eNb_txsF0", PHY_vars_eNb[0]->lte_eNB_common_vars.txdataF[eNb_id_secsys][0],lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti*10,1,1);
    write_output("eNb_txsigF1.m","eNb_txsF1", PHY_vars_eNb[0]->lte_eNB_common_vars.txdataF[eNb_id_secsys][1],lte_frame_parms->ofdm_symbol_size*lte_frame_parms->symbols_per_tti*10,1,1);
  }
#endif //SECONDARY_SYSTEM
  
  /*
  // optional: read rx_frame from file
  if ((rx_frame_file = fopen("rx_frame.dat","r")) == NULL)
    {
      printf("[openair][CHBCH_TEST][INFO] Cannot open rx_frame.m data file\n");
      exit(0);
    }

  fclose(rx_frame_file);
  */
    } //for(slot...
  } //for(mac_xface->frame...

#ifdef SECONDARY_SYSTEM
  if (!fopen(channel_output,"r")) {
    channel_file = fopen(channel_output,"w");
    if (channel_file) {
      fprintf(channel_file,"%d\n",1);
      for (aa=0; aa<6; aa++) { //loop over channel index
	for (i=0; i<lte_frame_parms->nb_antennas_rx; i++) {
	  for (j=0; j<lte_frame_parms->nb_antennas_tx; j++) {
	    for (l=0; l<channel_length; l++) {
	      fprintf(channel_file,"%f %f\n",ch_ar[aa][i + j*2][l].r, ch_ar[aa][i + j*2][l].i);
	    }
	  }
	}
      }
      fclose(channel_file);
    }
  }
#endif

#ifdef IFFT_FPGA
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
#endif 

  for (i=0;i<2;i++) {
    free(s_re[i]);
    free(s_im[i]);
    free(r_re[i]);
    free(r_im[i]);
  }
  free(s_re);
  free(s_im);
  free(r_re);
  free(r_im);

#ifdef SECONDARY_SYSTEM
  for (i=0;i<2;i++) {
    for (j=0; j<2; j++) {
      free(s_re_ext[j][i]);
      free(s_im_ext[j][i]);
      free(r_re_ext[j][i]);
      free(r_im_ext[j][i]);
    }
  }
  free(s_re_ext[j]);
  free(s_im_ext[j]);
  free(r_re_ext[j]);
  free(r_im_ext[j]);
  for (l=0; l<6; l++) {
    free(r_re_crossLink[l]);
    free(r_im_crossLink[l]);
  }
#endif //SECONDARY_SYSTEM
  
#ifndef SECONDARY_SYSTEM
  free(PHY_vars_eNb[0]);
  free(PHY_vars_UE[0]);
#else //SECONDARY_SYSTEM
  free(PHY_vars_eNb[0]);
  free(PHY_vars_eNb[1]);
  free(PHY_vars_UE[0]);
  free(PHY_vars_UE[1]);
#endif

  for (i = 0; i<4; i++)
    free(ch[i]);
  free(ch);
#ifdef SECONDARY_SYSTEM
  for (l=0; l<6; l++) {
    for (i = 0; i<4; i++)
      free(ch_ar[l][i]);
    free(ch_ar[l]);
  }
#endif //SECONDARY_SYSTEM

  lte_sync_time_free();

  return(n_errors);
}
