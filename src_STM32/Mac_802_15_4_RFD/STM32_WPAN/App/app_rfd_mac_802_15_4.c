/**
  ******************************************************************************
 * @file    app_rfd_mac_802_15_4.c
 * @author  MCD Application Team
 * @brief   Application based on MAC 802.15.4
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "stm_queue.h"

#include "utilities_common.h"

#include "app_entry.h"

#include "app_rfd_mac_802_15_4.h"

#include "802_15_4_mac_sap.h"
#include "app_rfd_mac_802_15_4_process.h"

#include "dbg_trace.h"
#include "shci.h"
#include "stm_logging.h"

////////////////////////////////////
#include "stm_rng.h"
#include "commit.h"
#include "fips202.h"
#include "poly.h"
#include "elapsed_time.h"
///////////////////////////////////

#define DEMO_CHANNEL 			20

#define DS2_DEBUG

#define TIMER_KEYGEN_START 		0
#define TIMER_KEYGEN_STAGE_1 	1
#define TIMER_KEYGEN_STAGE_2 	2
#define TIMER_KEYGEN_STAGE_3 	3
#define TIMER_KEYGEN_FINAL		4
#define TIMER_KEYGEN_TOTAL		5

#define TIMER_SIGN_START 		6
#define TIMER_SIGN_STAGE_1 		7
#define TIMER_SIGN_STAGE_2 		8
#define TIMER_SIGN_STAGE_3 		9
#define TIMER_SIGN_FINAL		10
#define TIMER_SIGN_TOTAL		11
#define TIMER_SIGN_ITERATION	12

/* Private function prototypes -----------------------------------------------*/


uint8_t xorSign( const char * pmessage, uint32_t message_len);

extern MAC_dataInd_t      g_DataInd_rx;


static void APP_RFD_MAC_802_15_4_TraceError(char * pMess, uint32_t ErrCode);
static void APP_RFD_MAC_802_15_4_Config(void);

////////////////////////////////////////////////////////////////////////
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Reset(void);
static void APP_RFD_MAC_802_15_4_DS2_Sign_Reset(void);

static void APP_RFD_MAC_802_15_4_DS2_Abort(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Start(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Final(void);

static void APP_RFD_MAC_802_15_4_DS2_Sign_Start(void);
static void APP_RFD_MAC_802_15_4_DS2_Sign_Stage_1(void);
static void APP_RFD_MAC_802_15_4_DS2_Sign_Stage_2(void);
static void APP_RFD_MAC_802_15_4_DS2_Sign_Stage_3(void);
static void APP_RFD_MAC_802_15_4_DS2_Sign_Final(void);



////////////////////////////////////////////////////////////////////////
uint8_t	 			g_AppState = DS2_IDLE;
static uint8_t 		rfBuffer[256];
static DS2_Packet 	g_msg_buffer = {0};
static DS2_Party 	g_DS2_Data = {0};


//private key
static uint8_t 		private_seed[SEED_BYTES] = {0};
static poly_t 		t0[K] = {0};
static poly_t 		s1[L];
static poly_t 		s2[K];

static uint8_t		tr[SEED_BYTES] = {0};
//public key
static uint8_t 		g_rho[SEED_BYTES] = {0};
static poly_t		t1[K] = {0};
static poly_t		t[K] = {0};
static poly_t		A[K][L] = {0};

//signature
static uint8_t		rej = 1;
static uint32_t		iterations = 0;
static uint32_t		nonce_y1 = 0;
static uint32_t		nonce_y2 = 0;
static uint8_t 		y_seed[SEED_BYTES] = {0};
static uint8_t 		r_seed[SEED_BYTES] = {0};
static uint8_t 		ck_seed[SEED_BYTES] = {0};
static poly_t		y1[L] = {0};
static poly_t		y2[K] = {0};
static poly_t		z1[L] = {0};
static poly_t		z2[K] = {0};
static uint8_t		c[SEED_BYTES] = {0};
static poly_t		poly_c = {0};
static poly_t		w[K] = {0};
static poly_t		Fi[K][K] = {0};

//global
static uint16_t 	g_packet_cnt = 0;
static uint16_t     g_panId             = 0x1AAA;
static uint16_t     g_coordShortAddr    = 0x1122;
static uint8_t      g_dataHandle        = 0x02;
static long long    g_extAddr           = 0xACDE480000000002;
static uint8_t      g_channel           = DEMO_CHANNEL;
static uint8_t      g_channel_page      = 0x00;

MAC_callbacks_t macCbConfig ;
MAC_associateCnf_t g_MAC_associateCnf;

__IO ITStatus CertifOutputPeripheralReady = SET;


/* Functions Definition ------------------------------------------------------*/

void APP_RFD_MAC_802_15_4_Init( APP_MAC_802_15_4_InitMode_t InitMode, TL_CmdPacket_t* pCmdBuffer)
{
	APP_DBG("RFD MAC APP - APP_RFD_MAC_802_15_4_Init");
  /* Register cmdbuffer */
  APP_ENTRY_RegisterCmdBuffer(pCmdBuffer);

  /* Init config buffer and call TL_THREAD_Init */
  APP_ENTRY_TL_MAC_802_15_4_Init();

  /* Send MAC_802_15_4 start system cmd to RF Core */
  SHCI_C2_MAC_802_15_4_Init();

  /* Register task */
  /* Create the different tasks */
  UTIL_SEQ_RegTask( 1<<CFG_TASK_MSG_FROM_RF_CORE, UTIL_SEQ_RFU, APP_ENTRY_ProcessMsgFromRFCoreTask);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_RFD, UTIL_SEQ_RFU,APP_RFD_MAC_802_15_4_SetupTask);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_ABORT, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_Abort);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_START, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Start);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_1, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_2, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_3, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_FINAL, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Final);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_START, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_Sign_Start);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_STAGE_1, UTIL_SEQ_RFU,APP_RFD_MAC_802_15_4_DS2_Sign_Stage_1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_STAGE_2, UTIL_SEQ_RFU,APP_RFD_MAC_802_15_4_DS2_Sign_Stage_2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_STAGE_3, UTIL_SEQ_RFU,APP_RFD_MAC_802_15_4_DS2_Sign_Stage_3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_FINAL, UTIL_SEQ_RFU,APP_RFD_MAC_802_15_4_DS2_Sign_Final);

  /* Configuration MAC 802_15_4 */
  APP_RFD_MAC_802_15_4_Config();


  /*Start Main Node - RFD Task*/
  UTIL_SEQ_SetTask( 1<< CFG_TASK_RFD, CFG_SCH_PRIO_0 );

}


void APP_RFD_MAC_802_15_4_NodeSrvTask(void)
{
	APP_DBG("RFD MAC APP - APP_RFD_MAC_802_15_4_NodeSrvTask");
  APP_DBG("Srv task :");
}

void APP_RFD_MAC_802_15_4_NodeDataTask(void)
{
	APP_DBG("RFD MAC APP - APP_RFD_MAC_802_15_4_NodeDataTask");
}


void APP_RFD_MAC_802_15_4_SetupTask(void)
{
	APP_DBG("RFD MAC APP - APP_RFD_MAC_802_15_4_SetupTask");
  MAC_Status_t MacStatus = MAC_ERROR;

  MAC_resetReq_t    ResetReq;
  MAC_setReq_t      SetReq;
  uint8_t PIB_Value = 0x00;

  MAC_associateReq_t AssociateReq;



  APP_DBG("RFD MAC APP - Startup\0");

  /* Reset RFD Device */
  /* Reset MAC */
  memset(&ResetReq,0x00,sizeof(MAC_resetReq_t));
  ResetReq.set_default_PIB = TRUE;

  MacStatus = MAC_MLMEResetReq( &ResetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("RFD MAC - Reset Fails\0");
    return;
  }
  /* Wait for Reset Confirmation */
  UTIL_SEQ_WaitEvt( 1U<< CFG_EVT_DEVICE_RESET_CNF );
  APP_DBG("RFD MAC APP - Reset CNF Received\0");

  /* Set Device Extended Address */
  APP_DBG("RFD MAC APP - Set Extended Address\0");
  memset(&SetReq,0x00,sizeof(MAC_setReq_t));
  SetReq.PIB_attribute = g_MAC_EXTENDED_ADDRESS_c;
  SetReq.PIB_attribute_valuePtr = (uint8_t*)&g_extAddr;

  MacStatus = MAC_MLMESetReq( &SetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("RFD MAC - Set Extended Addr Fails\0");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U<< CFG_EVT_SET_CNF );
  APP_DBG("RFD MAC APP - Set Extended Address CNF Received\0");
  BSP_LED_On(LED1);

  /* Set Association Request */
  APP_DBG("RFD MAC APP - Association REQ\0");

  AssociateReq.channel_number   = g_channel;
  AssociateReq.channel_page     = g_channel_page;
  AssociateReq.coord_addr_mode  = g_SHORT_ADDR_MODE_c;
  memcpy(AssociateReq.coord_address.a_short_addr,&g_coordShortAddr,0x02);
  AssociateReq.capability_information = 0x80;
  memcpy(AssociateReq.a_coord_PAN_id,&g_panId,0x02);
  AssociateReq.security_level = 0x00;

  MacStatus = MAC_MLMEAssociateReq( &AssociateReq);


  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("RFD MAC - Association Req Fails\0");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_ASSOCIATE_CNF );
  APP_DBG("RFD MAC APP - Set Association Permit CNF Received\0");
  if(g_MAC_associateCnf.status != MAC_SUCCESS)
  {
    APP_DBG("RFD MAC APP - ASSOCIATION FAILS : status %x\0", g_MAC_associateCnf.status);
    return;
  }

  /* Set allocated Short Address : */
  /* Set Device Short Address */
    APP_DBG("RFD MAC APP - Set Short Address\0");
    memset(&SetReq,0x00,sizeof(MAC_setReq_t));
    SetReq.PIB_attribute = g_MAC_SHORT_ADDRESS_c;
    SetReq.PIB_attribute_valuePtr = (uint8_t*)&g_MAC_associateCnf.a_assoc_short_address;
    MacStatus = MAC_MLMESetReq( &SetReq );
    if ( MAC_SUCCESS != MacStatus ) {
      APP_DBG("RFD MAC - Set Short Addr Fails\0");
      return;
    }
    UTIL_SEQ_WaitEvt( 1U << CFG_EVT_SET_CNF );
    APP_DBG("RFD MAC APP - Set Short Address CNF Received\0");

    /* Set RxOnWhenIdle */
    APP_DBG("FFD MAC APP - Set RX On When Idle");
    memset(&SetReq,0x00,sizeof(MAC_setReq_t));
    SetReq.PIB_attribute = g_MAC_RX_ON_WHEN_IDLE_c;
    PIB_Value = g_TRUE;
    SetReq.PIB_attribute_valuePtr = &PIB_Value;

    MacStatus = MAC_MLMESetReq( &SetReq );
    if ( MAC_SUCCESS != MacStatus ) {
      APP_DBG("FFD MAC - Set Rx On When Idle Fails");
      return;
    }
    UTIL_SEQ_WaitEvt( 1U << CFG_EVT_SET_CNF );
    APP_DBG("FFD MAC APP - Set RX On When Idle CNF Received");


    BSP_LED_On(LED2);

    g_msg_buffer.src_node_id = DS2_NODE_ID;
    g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
    g_msg_buffer.msg_code = DS2_COORDINATOR_HELLO;
    g_msg_buffer.packet_length = 4;

    APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
}

void APP_RFD_MAC_802_15_4_SendData(uint16_t dst_addr, DS2_Packet* data)
{
  MAC_Status_t MacStatus = MAC_ERROR;

  BSP_LED_On(LED3);
  MAC_dataReq_t DataReq;
  //APP_DBG("RFD MAC APP - Send Data to Coordinator\0");
  DataReq.src_addr_mode = g_SHORT_ADDR_MODE_c;
  DataReq.dst_addr_mode = g_SHORT_ADDR_MODE_c;

  memcpy(DataReq.a_dst_PAN_id,&g_panId,0x02);
  memcpy(DataReq.dst_address.a_short_addr,(uint8_t*)&dst_addr,0x02);

  uint8_t data_len = data->packet_length;
  DataReq.msdu_handle = g_dataHandle++;
  DataReq.ack_Tx = FALSE;
  DataReq.GTS_Tx = FALSE;
  memcpy(&rfBuffer,(uint8_t*)data,data_len);
  rfBuffer[data_len] = xorSign((char*)data,data_len);
  DataReq.msduPtr = (uint8_t*) rfBuffer;
  DataReq.msdu_length = data_len+1;
  DataReq.security_level = 0x00;
  MacStatus = MAC_MCPSDataReq( &DataReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("RFD MAC - Data Req Fails\0");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_DATA_DATA_CNF );
  BSP_LED_Off(LED3);
  //APP_DBG("RFD MAC APP - DATA CNF Received\0");
}
/**
  * @brief  Trace the error or the warning reported.
  * @param  ErrId :
  * @param  ErrCode
  * @retval None
  */

void APP_RFD_MAC_802_15_4_Error(uint32_t ErrId, uint32_t ErrCode)
{

  switch(ErrId)
  {
  case ERR_APPLI_REC_MULTI_MSG_FROM_RFCore :
    APP_RFD_MAC_802_15_4_TraceError("ERROR : ERR_REC_MULTI_MSG_FROM_RFCore ",ErrCode);
    break;
  case ERR_INTERFACE_IPCC_SEND_ACK_TO_RFCore :
    APP_RFD_MAC_802_15_4_TraceError("ERROR : ERR_IPCC_SEND_ACK_TO_RFCore ",ErrCode);
    break;
  default :
    APP_RFD_MAC_802_15_4_TraceError("ERROR Unknown ",0);
    break;
  }
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void APP_RFD_MAC_802_15_4_Config()
{
	APP_DBG("RFD MAC APP - APP_RFD_MAC_802_15_4_Config");
  APP_DBG("configure RFD MAC 802.15.4 - 2\0");
    /* Register MAC 802.15.4 callback functions */

  memset(&macCbConfig,0x00,sizeof(MAC_callbacks_t));

  /* Mac Call Back Initialization */
  macCbConfig.mlmeResetCnfCb = APP_MAC_mlmeResetCnfCb;
  macCbConfig.mlmeScanCnfCb = APP_MAC_mlmeScanCnfCb;
  macCbConfig.mlmeAssociateCnfCb = APP_MAC_mlmeAssociateCnfCb;
  macCbConfig.mlmeAssociateIndCb = APP_MAC_mlmeAssociateIndCb;
  macCbConfig.mlmeBeaconNotifyIndCb = APP_MAC_mlmeBeaconNotifyIndCb;
  macCbConfig.mlmeCalibrateCnfCb = APP_MAC_mlmeCalibrateCnfCb;
  macCbConfig.mlmeCommStatusIndCb = APP_MAC_mlmeCommStatusIndCb;
  macCbConfig.mlmeDisassociateCnfCb = APP_MAC_mlmeDisassociateCnfCb;
  macCbConfig.mlmeDisassociateIndCb = APP_MAC_mlmeDisassociateIndCb;
  macCbConfig.mlmeDpsCnfCb = APP_MAC_mlmeDpsCnfCb;
  macCbConfig.mlmeDpsIndCb = APP_MAC_mlmeDpsIndCb;
  macCbConfig.mlmeGetCnfCb = APP_MAC_mlmeGetCnfCb;
  macCbConfig.mlmeGtsCnfCb = APP_MAC_mlmeGtsCnfCb;
  macCbConfig.mlmeGtsIndCb = APP_MAC_mlmeGtsIndCb;
  macCbConfig.mlmeOrphanIndCb = APP_MAC_mlmeOrphanIndCb;
  macCbConfig.mlmePollCnfCb = APP_MAC_mlmePollCnfCb;
  macCbConfig.mlmeRxEnableCnfCb = APP_MAC_mlmeRxEnableCnfCb;
  macCbConfig.mlmeSetCnfCb = APP_MAC_mlmeSetCnfCb;
  macCbConfig.mlmeSoundingCnfCb = APP_MAC_mlmeSoundingCnfCb;
  macCbConfig.mlmeStartCnfCb = APP_MAC_mlmeStartCnfCb;
  macCbConfig.mlmeSyncLossIndCb = APP_MAC_mlmeSyncLossIndCb;
  macCbConfig.mcpsDataIndCb = APP_MAC_mcpsDataIndCb;
  macCbConfig.mcpsDataCnfCb = APP_MAC_mcpsDataCnfCb;
  macCbConfig.mcpsPurgeCnfCb = APP_MAC_mcpsPurgeCnfCb;
  macCbConfig.mlmePollIndCb =  APP_MAC_mlmePollIndCb;
}

/**
  * @brief  Warn the user that an error has occurred.In this case,
  *         the LEDs on the Board will start blinking.
  *
  * @param  pMess  : Message associated to the error.
  * @param  ErrCode: Error code associated to the module (OpenThread or other module if any)
  * @retval None
  */
static void APP_RFD_MAC_802_15_4_TraceError(char * pMess, uint32_t ErrCode)
{
  APP_DBG(pMess);
  while(1 == 1)
  {
    BSP_LED_Toggle(LED1);
    HAL_Delay(500);
    BSP_LED_Toggle(LED2);
    HAL_Delay(500);
    BSP_LED_Toggle(LED3);
    HAL_Delay(500);
  }
}


/************** APP RFD ********************/

/* Private variables ---------------------------------------------------------*/
/** @defgroup APP RFD private variables
 * @{
 */

/**
 * @}
 */

/* Global variables ----------------------------------------------------------*/
/** @defgroup APP RFD Global variable
 * @{
 */


/**
 * @}
 */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup APP RFD private function prototypes
 * @{
 */


/**
  * @brief  compute  simple xor signature of the data to transmit
  *
  * @param  pmessage   : Message to sign.
  * @param  message_len: Message Len
  * @retval Message Signature
  */

uint8_t xorSign( const char * pmessage, uint32_t message_len)
{
  uint8_t seed = 0x00;
  for (uint8_t i=0x00;i<message_len;i++)
    seed = (uint8_t)pmessage[i]^seed;
  return seed; 
}

/**
 * @}
 */


/* Private Functions Definition ------------------------------------------------------*/
/** @defgroup APP RFD Private function
 * @{
 */

static void APP_RFD_MAC_802_15_4_DS2_SendKeyGen_Start(void)
{
	APP_RFD_MAC_802_15_4_DS2_KeyGen_Reset();

	g_msg_buffer.src_node_id = DS2_NODE_ID;
	g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
	g_msg_buffer.msg_code = DS2_KEYGEN_START_TASK;
	g_msg_buffer.packet_length = 4;
	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

	APP_RFD_MAC_802_15_4_DS2_KeyGen_Start();
}

static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Start(void)
{
	switch(g_AppState){
	case DS2_READY:
#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 - KEYGEN - STAGE 0");
#endif
		g_AppState = DS2_KEYGEN_START_IDLE;
	case DS2_KEYGEN_START_IDLE:

		APP_RFD_MAC_802_15_4_DS2_KeyGen_Reset();

		elapsed_time_start(TIMER_KEYGEN_TOTAL);
		elapsed_time_start(TIMER_KEYGEN_START);

		uint32_t* seed_ptr = (uint32_t*)g_DS2_Data.pi_val;

		//generate pi
		for(int i = 0; i < (SEED_BYTES / sizeof(uint32_t)); i++)
		{
			RNG_GenerateRandomInt(seed_ptr);
			seed_ptr++;
		}
		// generate pi commit
		h1(g_DS2_Data.pi_val, DS2_NODE_ID, g_DS2_Data.pi_commit);

		elapsed_time_stop(TIMER_KEYGEN_START);

#ifdef DS2_DEBUG

        uint8_t pix = xorSign((char*)g_DS2_Data.pi_val, sizeof(g_DS2_Data.pi_val));
        APP_DBG("RFD DS2 - KEYGEN - pi = rand() = %ld", pix);

        uint8_t gix = xorSign((char*)g_DS2_Data.pi_commit, sizeof(g_DS2_Data.pi_commit));
        APP_DBG("RFD DS2 - KEYGEN - gi = h1(pi) = %ld", gix);
#endif

		g_msg_buffer.src_node_id = DS2_NODE_ID;
		g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.msg_code = DS2_Pi_COMMIT;
		g_msg_buffer.packet_length = DS2_HEADER_LEN + DS2_Pi_COMMIT_SIZE; //send g_i - size 64 byte
		g_msg_buffer.data_offset = 0;

		memcpy((uint8_t*)g_msg_buffer.data, g_DS2_Data.pi_commit, DS2_Pi_COMMIT_SIZE);

		APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
		g_AppState = DS2_KEYGEN_START_END;
		break;
	default:
		APP_DBG("RFD DS2 - KYEGEN - ERROR: STAGE 0 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}


static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void)
{
	switch(g_AppState){
	case DS2_KEYGEN_START_END:

#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 - KEYGEN - STAGE 1");
#endif
		g_AppState = DS2_KEYGEN_STAGE_1_IDLE;
	case DS2_KEYGEN_STAGE_1_IDLE:

		memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));

		g_msg_buffer.src_node_id = DS2_NODE_ID;
		g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.msg_code = DS2_Pi_VALUE;
		g_msg_buffer.packet_length = DS2_HEADER_LEN + DS2_Pi_VALUE_SIZE;

		memcpy((uint8_t*)g_msg_buffer.data, g_DS2_Data.pi_val, DS2_Pi_VALUE_SIZE);
		g_msg_buffer.data_offset = 0;

		APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
		g_AppState = DS2_KEYGEN_STAGE_1_END;
		break;
	default:
		APP_DBG("RFD DS2 - KYEGEN - ERROR: STAGE 1 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}


static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void)
{
	poly_t s1_[L];

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_1_END:
#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 - KEYGEN - STAGE 2");
#endif
		g_AppState = DS2_KEYGEN_STAGE_2_IDLE;
	case DS2_KEYGEN_STAGE_2_IDLE:

		elapsed_time_start(TIMER_KEYGEN_STAGE_2);

		memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));
		DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;

		//save result rho
		memcpy(g_rho, (uint8_t*)packet_ptr->data, SEED_BYTES);

		//generate A
		poly_uniform(g_rho, K * L, 0, &A[0][0]);

		//generate private seed
		uint32_t* seed_ptr = (uint32_t*)private_seed;
		for(int i = 0; i < (SEED_BYTES / sizeof(uint32_t)); i++)
		{
			RNG_GenerateRandomInt(seed_ptr);
			seed_ptr++;
		}

		//generate private key
		poly_eta(private_seed, 0, L, s1);
		poly_eta(private_seed, L, K, s2);

		// Compute ti = (A | I) * s_1 + s_2
		poly_copy(s1, L, s1_);

		poly_ntt(s1_, L);
		poly_product(A, s1_, t1);
		poly_reduce(t1, K);
		poly_invntt_tomont(t1, K);

		poly_add(t1, s2, K, t1);
		poly_addq(t1, K);
		poly_power2round(t1, K, t0);

		poly_pack(T1_BITS, t1, K, g_DS2_Data.ti_val);

		//generate ti commit
		h2(g_DS2_Data.ti_val, DS2_NODE_ID, g_DS2_Data.ti_commit);

		elapsed_time_stop(TIMER_KEYGEN_STAGE_2);

#ifdef DS2_DEBUG

		uint8_t rhox =  xorSign((char*)g_rho, sizeof(g_rho));
		APP_DBG("RFD DS2 - KEYGEN - rho = h(p0, p1, ... pi) = %ld", rhox);

		APP_DBG("RFD DS2 - KEYGEN - A = H(rho) = %ld", A[1][1].coeffs[_N-1]);

		APP_DBG("RFD DS2 - KEYGEN - ti = (A | I) * s_1 + s_2 = %ld", t1[1].coeffs[_N-1]);

		uint8_t gtix =  xorSign((char*)g_DS2_Data.ti_commit, sizeof(g_DS2_Data.ti_commit));
		APP_DBG("RFD DS2 - KEYGEN - gti = h2(ti) = %ld", gtix);
#endif

		//send commit ti
		g_msg_buffer.src_node_id = DS2_NODE_ID;
		g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.msg_code = DS2_Ti_COMMIT;
		g_msg_buffer.packet_length = DS2_HEADER_LEN + DS2_Ti_COMMIT_SIZE;

		memcpy((char*)&g_msg_buffer.data, g_DS2_Data.ti_commit, DS2_Ti_COMMIT_SIZE);
		g_msg_buffer.data_offset = 0;

		APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);

		g_AppState = DS2_KEYGEN_STAGE_2_END;
		break;
	default:
		APP_DBG("RFD DS2 - KYEGEN - ERROR: STAGE 2 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}


static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void)
{
	switch(g_AppState){
	case DS2_KEYGEN_STAGE_2_END:

#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 - KEYGEN - STAGE 3");
#endif
		g_AppState = DS2_KEYGEN_STAGE_3_IDLE;
	case DS2_KEYGEN_STAGE_3_IDLE:

		memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));
		int i = 0;

		g_msg_buffer.src_node_id = DS2_NODE_ID;
		g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.msg_code = DS2_Ti_VALUE;
		g_msg_buffer.packet_length = DS2_HEADER_LEN + (DS2_MAX_DATA_LEN * 4);
		g_msg_buffer.data_offset = 0;
		uint8_t packet_num = DS2_Ti_VALUE_SIZE / (DS2_MAX_DATA_LEN * 4);
		uint8_t last_data_len = DS2_Ti_VALUE_SIZE % (DS2_MAX_DATA_LEN * 4);

		for(i = 0; i < packet_num; i++){
			g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);

			memcpy((char*)g_msg_buffer.data, &g_DS2_Data.ti_val[g_msg_buffer.data_offset], (DS2_MAX_DATA_LEN * 4));

			APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
		}

		if(last_data_len > 0){
			g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);
			g_msg_buffer.packet_length = last_data_len + DS2_HEADER_LEN;

			memcpy((char*)g_msg_buffer.data, &g_DS2_Data.ti_val[g_msg_buffer.data_offset], last_data_len);

			APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
		}
		g_AppState = DS2_KEYGEN_STAGE_3_END;
		break;
	default:
		APP_DBG("RFD DS2 - KYEGEN - ERROR: STAGE 3 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Final(void)
{

	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	static uint8_t t1_packed[DS2_Ti_VALUE_SIZE] = {0};

	switch(g_AppState){
		case DS2_KEYGEN_STAGE_3_END:
#ifdef DS2_DEBUG
			APP_DBG("RFD DS2 - KEYGEN - STAGE 4");
#endif
			g_AppState = DS2_KEYGEN_FINAL_IDLE;
			g_packet_cnt = 0;
		case DS2_KEYGEN_FINAL_IDLE:

			g_packet_cnt++;
			memcpy(&t1_packed[offset], (uint8_t*)packet_ptr->data, data_size);
			//APP_DBG("DS2 - PACKET: %d SIZE: %d OFFSET: %d", g_packet_cnt, data_size, offset);
			//all packets from node src_id were received
			if(g_packet_cnt*DS2_MAX_DATA_LEN*4 > DS2_Ti_VALUE_SIZE){

				elapsed_time_start(TIMER_KEYGEN_FINAL);

				//unpack t1
				poly_unpack(T1_BITS, t1_packed, K, 0, t);

				//calculate tr
			    keccak_state_t state;
			    keccak_init(&state);
			    shake128_absorb(&state, g_rho, SEED_BYTES);
			    shake128_absorb(&state, (uint8_t*) t, sizeof(t));
			    shake128_finalize(&state);
			    shake128_squeeze(&state, SEED_BYTES, tr);

			    elapsed_time_stop(TIMER_KEYGEN_FINAL);

#ifdef DS2_DEBUG

				APP_DBG("RFD DS2 - KEYGEN - t1 = sum(t0, t1, ... ti) = %ld", t[1].coeffs[_N-1]);

			    uint8_t trx =  xorSign((char*)tr, sizeof(tr));
			    APP_DBG("RFD DS2 - KEYGEN - tr = h(rho, t1) = %ld", trx);
#endif

			    g_AppState = DS2_READY;
			    elapsed_time_stop(TIMER_KEYGEN_TOTAL);

			    APP_DBG("RFD DS2 - KYEGEN - STAGE 0 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_START));
			    APP_DBG("RFD DS2 - KYEGEN - STAGE 1 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_STAGE_1));
			    APP_DBG("RFD DS2 - KYEGEN - STAGE 2 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_STAGE_2));
			    APP_DBG("RFD DS2 - KYEGEN - STAGE 3 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_STAGE_3));
			    APP_DBG("RFD DS2 - KYEGEN - STAGE 4 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_FINAL));
			    APP_DBG("RFD DS2 - KYEGEN - TOTAL   TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_TOTAL));
			}
			break;
		default:
			APP_DBG("RFD DS2 - KYEGEN - ERROR: STAGE 4 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
			break;
		}
}

////////////////////////////////////////////////////////

static void APP_RFD_MAC_802_15_4_DS2_SendSign_Start(void)
{
	APP_RFD_MAC_802_15_4_DS2_Sign_Reset();

	g_msg_buffer.src_node_id = DS2_NODE_ID;
	g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
	g_msg_buffer.msg_code = DS2_SIGN_START_TASK;
	g_msg_buffer.packet_length = 4;
	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

	APP_RFD_MAC_802_15_4_DS2_Sign_Start();
}



static void APP_RFD_MAC_802_15_4_DS2_Sign_Start(void)
{
	poly_t y1_[L] = {0};

	static char msg[] = "test message\0";
	static uint32_t msg_len = sizeof(msg);

	uint32_t* y_seed_ptr;
	uint32_t* r_seed_ptr;
	switch(g_AppState){
	case DS2_READY:

#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 -- SIGN -- STAGE 0");
#endif

		iterations++;
		elapsed_time_start(TIMER_SIGN_TOTAL);

		g_AppState = DS2_SIGN_START_IDLE;

	case DS2_SIGN_START_IDLE:

		elapsed_time_start(TIMER_SIGN_START);
		//generate r and y seed
		y_seed_ptr = (uint32_t*)y_seed;
		r_seed_ptr = (uint32_t*)r_seed;
		for(int i = 0; i < (SEED_BYTES / sizeof(uint32_t)); i++)
		{
			RNG_GenerateRandomInt(y_seed_ptr);
			y_seed_ptr++;

			RNG_GenerateRandomInt(r_seed_ptr);
			r_seed_ptr++;
		}

	    keccak_state_t state;
	    keccak_init(&state);
	    shake256_absorb(&state, msg, msg_len);
	    shake256_absorb(&state, tr, SEED_BYTES);
	    shake256_finalize(&state);
	    shake256_squeeze(&state, SEED_BYTES, ck_seed);

		//generate y1 and y2
		poly_normal(y_seed, nonce_y1, SIGMA, L, y1);
		nonce_y1 += L;
		poly_normal(y_seed, nonce_y2, SIGMA, K, y2);
		nonce_y2 += K;

		// Compute w_n = (A | I) * y_n
		poly_copy(y1, L, y1_);
		poly_ntt(y1_, L);

		poly_product(A, y1_, w);
		poly_reduce(w, K);
		poly_invntt_tomont(w, K);

		poly_add(w, y2, K, w);
		poly_freeze(w, K);

#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 -- SIGN -- GENERATE COMMITMENT com_i ...");
#endif
		poly_gen_commit(ck_seed, r_seed, Fi);
		poly_add(&Fi[1], w, K, &Fi[1]);

		poly_freeze((poly_t*)Fi, K*K);

		poly_pack(TC_L, (poly_t*) Fi, K * K, g_DS2_Data.fi_commit);

		elapsed_time_stop(TIMER_SIGN_START);

#ifdef DS2_DEBUG
	    uint8_t yx =  xorSign((char*)y_seed, sizeof(y_seed));
	    APP_DBG("RFD DS2 -- SIGN -- y_seed = rand() = %ld", yx);

	    uint8_t rx =  xorSign((char*)r_seed, sizeof(r_seed));
	    APP_DBG("RFD DS2 -- SIGN -- r_seed = rand() = %ld", rx);

	    uint8_t ckx =  xorSign((char*)ck_seed, sizeof(ck_seed));
	    APP_DBG("RFD DS2 -- SIGN -- ck_seed = h3(tr, msg) = %ld", ckx);
	    //uint8_t wx =  xorSign((char*)w, sizeof(w));
	    APP_DBG("RFD DS2 -- SIGN -- w_n = (A | I) * y_n = %ld", w[1].coeffs[_N-1]);

	    APP_DBG("RFD DS2 -- SIGN -- com_i = ck*r + w = %ld", Fi[1][1].coeffs[_N-1]);
#endif

		memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));
		int i = 0;

		g_msg_buffer.src_node_id = DS2_NODE_ID;
		g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.msg_code = DS2_Fi_COMMIT;
		g_msg_buffer.packet_length = DS2_HEADER_LEN + (DS2_MAX_DATA_LEN * 4);
		g_msg_buffer.data_offset = 0;
		uint8_t packet_num = DS2_Fi_COMMIT_SIZE / (DS2_MAX_DATA_LEN * 4);
		uint8_t last_data_len = DS2_Fi_COMMIT_SIZE % (DS2_MAX_DATA_LEN * 4);

		for(i = 0; i < packet_num; i++){
			g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);

			memcpy((char*)g_msg_buffer.data, &g_DS2_Data.fi_commit[g_msg_buffer.data_offset], (DS2_MAX_DATA_LEN * 4));

			APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
		}

		if(last_data_len > 0){
			g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);
			g_msg_buffer.packet_length = last_data_len + DS2_HEADER_LEN;

			memcpy((char*)g_msg_buffer.data, &g_DS2_Data.fi_commit[g_msg_buffer.data_offset], last_data_len);

			APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
		}

		g_AppState = DS2_SIGN_START_END;
		break;
	default:
		APP_DBG("RFD DS2 -- SIGN -- ERROR: STAGE 0 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_RFD_MAC_802_15_4_DS2_Sign_Stage_1(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	poly_t cs1[L] = {0};
	poly_t cs2[K] = {0};

	poly_t t0_[K] = {0};

	switch(g_AppState){
	case DS2_SIGN_START_END:
		g_AppState = DS2_SIGN_STAGE_1_IDLE;
		g_packet_cnt = 0;

#ifdef DS2_DEBUG
		APP_DBG("RFD DS2 -- SIGN -- STAGE 1");
#endif

	case DS2_SIGN_STAGE_1_IDLE:

		elapsed_time_start(TIMER_SIGN_STAGE_1);

		memcpy(c, (uint8_t*)packet_ptr->data, data_size);
		poly_challenge(c, &poly_c);

        // z_n = c * s_n + y_n
        poly_copy(s1, L, cs1);
        poly_copy(s2, K, cs2);

        poly_ntt(cs1, L);
        poly_ntt(cs2, K);
        poly_ntt(&poly_c, 1);

        poly_mul_pointwise(cs1, &poly_c, L, cs1);
        poly_mul_pointwise(cs2, &poly_c, K, cs2);

        poly_reduce(cs1, L);
        poly_reduce(cs2, K);

        poly_invntt_tomont(cs1, L);
        poly_invntt_tomont(cs2, K);

        poly_add(cs1, y1, L, z1);
        poly_add(cs2, y2, K, z2);

        rej = poly_reject(z1, z2, cs1, cs2);

    	poly_copy(t0, K, t0_);
    	poly_ntt(t0_, K);
    	poly_mul_pointwise(t0_, &poly_c, K, t0_);

    	poly_reduce(t0_, K);
    	poly_invntt_tomont(t0_, K);

    	poly_sub(z2, t0_, K, z2);

        poly_center(z1, L);
        poly_center(z2, K);

        //check norm
        rej |= !poly_check_norm(z1, L, B);
        rej |= !poly_check_norm(z2, K, B);

	    poly_addq(z1, L);
	    poly_pack(TC_L, z1, L, g_DS2_Data.zi_1_val);

	    poly_addq(z2, K);
	    poly_pack(TC_L, z2, L, g_DS2_Data.zi_2_val);

        elapsed_time_stop(TIMER_SIGN_STAGE_1);

#ifdef DS2_DEBUG
	    uint8_t cx =  xorSign((char*)c, sizeof(c));
	    APP_DBG("RFD DS2 -- SIGN -- c = H(com, msg, pk) = %ld", cx);
	    APP_DBG("RFD DS2 -- SIGN -- z1 = cs1 + y1 = %ld", z1[1].coeffs[_N-1]);
	    APP_DBG("RFD DS2 -- SIGN -- z2 = cs2 + y2 = %ld", z2[1].coeffs[_N-1]);
#endif

        if(rej){
        	APP_RFD_MAC_802_15_4_DS2_Sign_Reset();

    		g_msg_buffer.src_node_id = DS2_NODE_ID;
    		g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
    		g_msg_buffer.msg_code = DS2_ERROR_Zi_REJECT;
    		g_msg_buffer.packet_length = 4;
    		APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

    		UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_SIGN_START, CFG_SCH_PRIO_0 );

    		g_AppState = DS2_READY;

    		APP_DBG("RFD DS2 -- SIGN -- STAGE 1 REJECTED");
    		return;
        } else {

    		g_msg_buffer.src_node_id = DS2_NODE_ID;
    		g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
    		g_msg_buffer.msg_code = DS2_Ri_VALUE;
    		g_msg_buffer.packet_length = DS2_HEADER_LEN;
    		g_msg_buffer.data_offset = 0;

    		memcpy((char*)&g_msg_buffer.data, r_seed, sizeof(r_seed));
    		APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
        }

        g_AppState = DS2_SIGN_STAGE_1_END;
		break;
	default:
		APP_DBG("RFD DS2 -- SIGN -- ERROR: STAGE 1 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_RFD_MAC_802_15_4_DS2_Sign_Stage_2(void)
{
	uint8_t packet_num = 0;
	uint8_t last_data_len = 0;
	switch(g_AppState){
	case DS2_SIGN_STAGE_1_END:
		g_AppState = DS2_SIGN_STAGE_2_IDLE;
		g_packet_cnt = 0;

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 2");
#endif
	case DS2_SIGN_STAGE_2_IDLE:

	    packet_num = DS2_Zi_1_VALUE_SIZE / (DS2_MAX_DATA_LEN * 4);
	    last_data_len = DS2_Zi_1_VALUE_SIZE % (DS2_MAX_DATA_LEN * 4);

	    g_msg_buffer.src_node_id = DS2_NODE_ID;
	    g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
	    g_msg_buffer.msg_code = DS2_Zi_1_VALUE;
	    g_msg_buffer.packet_length = DS2_HEADER_LEN + (DS2_MAX_DATA_LEN * 4);

	    int j = 0;
	    //APP_DBG("DS2 - SIGN STAGE 2 send z1");
	    for(j = 0; j < packet_num; j++){
	    	g_msg_buffer.data_offset = j * (DS2_MAX_DATA_LEN * 4);

	    	memcpy(g_msg_buffer.data, &g_DS2_Data.zi_1_val[g_msg_buffer.data_offset], (DS2_MAX_DATA_LEN * 4));

	    	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	    }
	    //APP_DBG("DS2 - SIGN STAGE 2 send z1 last");
	    if(last_data_len > 0){
	    	g_msg_buffer.data_offset = j * (DS2_MAX_DATA_LEN * 4);
	    	g_msg_buffer.packet_length = last_data_len + DS2_HEADER_LEN;

	    	memcpy(g_msg_buffer.data, &g_DS2_Data.zi_1_val[g_msg_buffer.data_offset], last_data_len);

	    	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	    }

	    g_AppState = DS2_SIGN_STAGE_2_END;
		break;
	default:
		APP_DBG("FFD DS2 -- SIGN -- ERROR: STAGE 2 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_RFD_MAC_802_15_4_DS2_Sign_Stage_3(void)
{
	uint8_t packet_num = 0;
	uint8_t last_data_len = 0;
	switch(g_AppState){
	case DS2_SIGN_STAGE_2_END:
		g_AppState = DS2_SIGN_STAGE_3_IDLE;
		g_packet_cnt = 0;

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 3");
#endif
	case DS2_SIGN_STAGE_3_IDLE:

	    packet_num = DS2_Zi_2_VALUE_SIZE / (DS2_MAX_DATA_LEN * 4);
	    last_data_len = DS2_Zi_2_VALUE_SIZE % (DS2_MAX_DATA_LEN * 4);

	    g_msg_buffer.src_node_id = DS2_NODE_ID;
	    g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
	    g_msg_buffer.msg_code = DS2_Zi_2_VALUE;
	    g_msg_buffer.packet_length = DS2_HEADER_LEN + (DS2_MAX_DATA_LEN * 4);

	    int j = 0;

	    for(j = 0; j < packet_num; j++){
	    	g_msg_buffer.data_offset = j * (DS2_MAX_DATA_LEN * 4);

	    	memcpy(g_msg_buffer.data, &g_DS2_Data.zi_2_val[g_msg_buffer.data_offset], (DS2_MAX_DATA_LEN * 4));

	    	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	    }

	    if(last_data_len > 0){
	    	g_msg_buffer.data_offset = j * (DS2_MAX_DATA_LEN * 4);
	    	g_msg_buffer.packet_length = last_data_len + DS2_HEADER_LEN;

	    	memcpy(g_msg_buffer.data, &g_DS2_Data.zi_2_val[g_msg_buffer.data_offset], last_data_len);

	    	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	    }

	    g_AppState = DS2_SIGN_STAGE_3_END;
		break;
	default:
		APP_DBG("FFD DS2 -- SIGN -- ERROR: STAGE 3 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_RFD_MAC_802_15_4_DS2_Sign_Final(void)
{
	/*DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;*/

	switch(g_AppState){
	case DS2_SIGN_STAGE_3_END:
		g_AppState = DS2_SIGN_FINAL_IDLE;
		g_packet_cnt = 0;

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 4");
#endif
	case DS2_SIGN_FINAL_IDLE:

		elapsed_time_stop(TIMER_SIGN_TOTAL);
		APP_DBG("RFD DS2 -- SIGN -- STAGE 0 TIMER:%ld",elapsed_time_max(TIMER_SIGN_START));
		APP_DBG("RFD DS2 -- SIGN -- STAGE 1 TIMER:%ld",elapsed_time_max(TIMER_SIGN_STAGE_1));
		APP_DBG("RFD DS2 -- SIGN -- STAGE 2 TIMER:%ld",elapsed_time_max(TIMER_SIGN_STAGE_2));
		APP_DBG("RFD DS2 -- SIGN -- STAGE 3 TIMER:%ld",elapsed_time_max(TIMER_SIGN_STAGE_3));
		APP_DBG("RFD DS2 -- SIGN -- STAGE 4 TIMER:%ld",elapsed_time_max(TIMER_SIGN_FINAL));
		APP_DBG("RFD DS2 -- SIGN -- TOTAL   TIMER:%ld",elapsed_time_max(TIMER_SIGN_TOTAL));
		APP_DBG("RFD DS2 -- SIGN -- ITERATIONS TOTAL:%ld",iterations);

		g_AppState = DS2_READY;
		break;
	default:
		APP_DBG("FFD DS2 -- SIGN -- ERROR: STAGE 4 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

////////////////////////////////////////////////////////

static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Reset(void)
{
	APP_RFD_MAC_802_15_4_DS2_Sign_Reset();
	memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));
	memset(t1, 0, sizeof(t1));
	memset(t0, 0, sizeof(t0));
	memset(s1, 0, sizeof(s1));
	memset(s2, 0, sizeof(s2));
	memset(t, 0, sizeof(t));
	memset(tr, 0, sizeof(tr));
	memset(g_rho, 0 , sizeof(g_rho));
	memset(private_seed, 0 , sizeof(private_seed));
	memset(A, 0 , sizeof(A));
	g_packet_cnt = 0;
	g_AppState = DS2_READY;
	iterations = 0;
}

static void APP_RFD_MAC_802_15_4_DS2_Sign_Reset(void)
{
	rej = 1;
	nonce_y1 = 0;
	nonce_y2 = 0;
	memset(y1, 0, sizeof(y1));
	memset(y2, 0, sizeof(y2));
	memset(z1, 0, sizeof(z1));
	memset(z2, 0, sizeof(z2));
	memset(w, 0, sizeof(w));
	memset(Fi, 0, sizeof(Fi));
	memset(y_seed, 0, sizeof(y_seed));
	memset(r_seed, 0, sizeof(r_seed));
	memset(c, 0 , sizeof(c));
	g_packet_cnt = 0;
	g_AppState = DS2_READY;

}

static void APP_RFD_MAC_802_15_4_DS2_Abort(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	APP_DBG("DS2 DATA ERROR - JOB ABORTED");
	switch(packet_ptr->msg_code){
	case DS2_ABORT:
		APP_DBG("DS2 ERROR CODE : %d", packet_ptr->msg_code);
		break;
	case DS2_ERROR_INVALID_NODE_ID:
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_INVALID_NODE_ID", packet_ptr->msg_code);
		break;
	case DS2_ERROR_NODE_ID_ALREADY_IN_USE:
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_NODE_ID_ALREADY_IN_USE", packet_ptr->msg_code);
		break;
	case DS2_ERROR_Ti_COMMIT:
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_Ti_COMMIT", packet_ptr->msg_code);
		break;
	case DS2_ERROR_Pi_COMMIT:
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_Pi_COMMIT", packet_ptr->msg_code);
		break;
	case DS2_ERROR_Fi_COMMIT:
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_Fi_COMMIT", packet_ptr->msg_code);
		break;
	case DS2_ERROR_Zi_REJECT:
		APP_RFD_MAC_802_15_4_DS2_Sign_Reset();
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_Zi_REJECT, NODE: %d", packet_ptr->msg_code, packet_ptr->src_node_id);
		UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_SIGN_START, CFG_SCH_PRIO_0 );
		return;
	}
	APP_RFD_MAC_802_15_4_DS2_KeyGen_Reset();

}

/**
 * @}
 */

/* Functions Definition ------------------------------------------------------*/
/** @defgroup APP RFD function prototypes
 * @{
 */


/**
 * @}
 */


