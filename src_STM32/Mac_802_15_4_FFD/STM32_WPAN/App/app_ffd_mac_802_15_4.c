/**
  ******************************************************************************
 * @file    app_ffd_mac_802_15_4.c
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
#include "app_ffd_mac_802_15_4.h"

#include "802_15_4_mac_sap.h"
#include "app_ffd_mac_802_15_4_process.h"

#include "dbg_trace.h"
#include "shci.h"
#include "stm_logging.h"


#define DEMO_CHANNEL 20


MAC_associateInd_t g_MAC_associateInd;




/* Private function prototypes -----------------------------------------------*/
static void APP_FFD_MAC_802_15_4_TraceError(char * pMess, uint32_t ErrCode);
static void APP_FFD_MAC_802_15_4_Config(void);

//////////////////////////////////////////////////////////////////////////////
static void APP_FFD_MAC_802_15_4_DS2_NewConnection(void);
static void APP_FFD_MAC_802_15_4_DS2_Abort(void);
static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void);
static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void);
static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void);
static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Final(void);

void APP_FFD_MAC_802_15_4_SendEcho(void);

static uint8_t xorSign( const char * pmessage, uint8_t message_len);



static DS2_Party 	g_Parties[DS2_MAX_PARTY_NUM] = {0};
static uint16_t 	g_packet_cnt[DS2_MAX_PARTY_NUM] = {0};
static DS2_Packet 	g_msg_buffer = {0};

static uint8_t 		g_rho[SEED_BYTES] = {0xAA};

static uint8_t	 	g_AppState = DS2_IDLE;

static uint16_t     g_panId             = 0x1AAA;
static uint16_t     g_coordShortAddr    = 0x1122;
static uint8_t      g_dataHandle        = 0x02;
static long long    g_extAddr           = 0xACDE480000000001;
static uint8_t      g_channel           = DEMO_CHANNEL;
static uint8_t      g_channel_page      = 0x00;

static uint8_t rfBuffer[256];
//////////////////////////////////////////////////////////////////////////////

MAC_callbacks_t macCbConfig ;


uint8_t g_srvSerReq;
uint8_t g_srvDataReq;


__IO ITStatus CertifOutputPeripheralReady = SET;


/* Functions Definition ------------------------------------------------------*/

void APP_FFD_MAC_802_15_4_Init( APP_MAC_802_15_4_InitMode_t InitMode, TL_CmdPacket_t* pCmdBuffer)
{

	APP_DBG("FFD MAC - APP_FFD_MAC_802_15_4_Init");
  /* Register cmdbuffer */
  APP_ENTRY_RegisterCmdBuffer(pCmdBuffer);

  /* Init config buffer and call TL_THREAD_Init */
  APP_ENTRY_TL_MAC_802_15_4_Init();

  /* Send MAC_802_15_4 start system cmd to RF Core */
  SHCI_C2_MAC_802_15_4_Init();

  /* Register task */
  /* Create the different tasks */
  UTIL_SEQ_RegTask( 1<<CFG_TASK_MSG_FROM_RF_CORE, UTIL_SEQ_RFU, APP_ENTRY_ProcessMsgFromRFCoreTask);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_FFD, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_SetupTask);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_SERVICE_COORD, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_CoordSrvTask);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DATA_COORD, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_CoordDataTask);


  UTIL_SEQ_RegTask( 1<<CFG_TASK_DATA_ECHO, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_SendEcho);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_NEW_CONNECTION, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_NewConnection);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_ABORT, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_Abort);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_1, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_2, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_3, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_FINAL, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Final);
  /* Configuration MAC 802_15_4 */
  APP_FFD_MAC_802_15_4_Config();


  /*Start Main Coordinator - FFD Task*/
  UTIL_SEQ_SetTask( 1<< CFG_TASK_FFD, CFG_SCH_PRIO_0 );

}


void APP_FFD_MAC_802_15_4_CoordSrvTask(void)
{
  MAC_Status_t MacStatus = MAC_ERROR;

  MAC_associateRes_t AssociateRes;
  uint16_t shortAssociationAddr = 0x3344;

  APP_DBG("Srv task :");

  switch (g_srvSerReq)
  {
  case CFG_ASSO_PENDING:
    APP_DBG("Srv task : Response to Association Indication");

    memcpy(AssociateRes.a_device_address,g_MAC_associateInd.a_device_address,0x08);
    memcpy(AssociateRes.a_assoc_short_address,&shortAssociationAddr,0x2);
    AssociateRes.security_level = 0x00;
    AssociateRes.status = MAC_SUCCESS;

    MacStatus = MAC_MLMEAssociateRes(&AssociateRes);
    if ( MAC_SUCCESS != MacStatus ) {
      APP_DBG("FFD MAC - Association Response Fails");
      return;
    }
    break;
  default:
    APP_DBG("Srv task : unknown Request");
  }
  g_srvSerReq = CFG_SRV_SER_REQ_NBR;
}

void APP_FFD_MAC_802_15_4_CoordDataTask(void)
{
  APP_DBG("Data task :");
  switch (g_srvDataReq)
  {
    case CFG_DATA_PENDING:
      BSP_LED_Toggle(LED3);
      APP_DBG("Data task : Data Indication Receive");
    default:
      APP_DBG("Data task : unknown Request");
  }
  g_srvDataReq = CFG_SRV_DATA_REQ_NBR;
}

void APP_FFD_MAC_802_15_4_SetupTask(void)
{
	//APP_DBG("FFD MAC - APP_FFD_MAC_802_15_4_SetupTask");
  MAC_Status_t MacStatus = MAC_ERROR;

  MAC_resetReq_t    ResetReq;
  MAC_setReq_t      SetReq;
  MAC_startReq_t    StartReq;

  long long extAddr    = g_extAddr;
  uint16_t shortAddr   = g_coordShortAddr;
  uint16_t panId       = g_panId;
  uint8_t channel      = g_channel;
  uint8_t PIB_Value = 0x00;
  
  int8_t tx_power_pib_value = 0;

  APP_DBG("Run FFD MAC 802.15.4 - 2 - FFD Startup");

  /* Reset FFD Device */
  /* Reset MAC */
  memset(&ResetReq,0x00,sizeof(MAC_resetReq_t));
  ResetReq.set_default_PIB = TRUE;

  MacStatus = MAC_MLMEResetReq( &ResetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("FFD MAC - Reset Fails");
    return;
  }
  /* Wait for Reset Confirmation */
  UTIL_SEQ_WaitEvt( 1U<< CFG_EVT_DEVICE_RESET_CNF );
  APP_DBG("FFD MAC APP - Reset CNF Received");

  /* Set Device Extended Address */
  APP_DBG("FFD MAC APP - Set Extended Address");
  memset(&SetReq,0x00,sizeof(MAC_setReq_t));
  SetReq.PIB_attribute = g_MAC_EXTENDED_ADDRESS_c;
  SetReq.PIB_attribute_valuePtr = (uint8_t*) &extAddr;
  MacStatus = MAC_MLMESetReq( &SetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("FFD MAC - Set Extended Addr Fails");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U<< CFG_EVT_SET_CNF );
  APP_DBG("FFD MAC APP - Set Extended Address CNF Received");


  /* Set Device Short Address */
  APP_DBG("FFD MAC APP - Set Short Address");
  memset(&SetReq,0x00,sizeof(MAC_setReq_t));
  SetReq.PIB_attribute = g_MAC_SHORT_ADDRESS_c;
  SetReq.PIB_attribute_valuePtr =(uint8_t*) &shortAddr;
  MacStatus = MAC_MLMESetReq( &SetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("FFD MAC - Set Short Addr Fails");
    return;
  }

  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_SET_CNF );
  APP_DBG("FFD MAC APP - Set Short Address CNF Received");

  /* Set Association Permit */
  APP_DBG("FFD MAC APP - Set Association Permit");

  memset(&SetReq,0x00,sizeof(MAC_setReq_t));
  SetReq.PIB_attribute = g_MAC_ASSOCIATION_PERMIT_c;
  PIB_Value = g_TRUE;
  SetReq.PIB_attribute_valuePtr = &PIB_Value;

  MacStatus = MAC_MLMESetReq( &SetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("FFD MAC - Set Association Permit Fails");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_SET_CNF );
  APP_DBG("FFD MAC APP - Set Association Permit CNF Received");



  /* Set Tx Power */

  APP_DBG("FFD MAC APP - Set TX Power");
  memset(&SetReq,0x00,sizeof(MAC_setReq_t));
  SetReq.PIB_attribute = g_PHY_TRANSMIT_POWER_c;
  tx_power_pib_value = 2;                       /* int8 dBm value in the range [-21;6] */
  SetReq.PIB_attribute_valuePtr = (uint8_t *)&tx_power_pib_value;

  MacStatus = MAC_MLMESetReq( &SetReq );
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("FFD MAC - Set Tx Power Fails");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_SET_CNF );
  
  
  /* Start Device */
  APP_DBG("FFD MAC APP - Start FFD Device");
  memset(&StartReq,0x00,sizeof(MAC_startReq_t));
  memcpy(StartReq.a_PAN_id,(uint8_t*)&panId,0x02);
  StartReq.channel_number   = channel;
  StartReq.beacon_order     = 0x0F;
  StartReq.superframe_order = 0x0F;
  StartReq.PAN_coordinator  = g_TRUE;
  StartReq.battery_life_extension = g_FALSE;

  MacStatus = MAC_MLMEStartReq( &StartReq);
  if ( MAC_SUCCESS != MacStatus ) {
    APP_DBG("FFD MAC - Set Association Permit Fails");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_DEVICE_STARTED_CNF );
  APP_DBG("FFD MAC APP - Start FFD Device CNF Received");


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
  /* Go in Echo loop */
  APP_DBG("FFD MAC APP - Ready to Handle Association Req and Receive Data");
  BSP_LED_Toggle(LED1);
}

/**
  * @brief  Trace the error or the warning reported.
  * @param  ErrId :
  * @param  ErrCode
  * @retval None
  */

void APP_FFD_MAC_802_15_4_Error(uint32_t ErrId, uint32_t ErrCode)
{

  switch(ErrId)
  {
  case ERR_APPLI_REC_MULTI_MSG_FROM_RFCore :
    APP_FFD_MAC_802_15_4_TraceError("ERROR : ERR_REC_MULTI_MSG_FROM_RFCore ",ErrCode);
    break;
  case ERR_INTERFACE_IPCC_SEND_ACK_TO_RFCore :
    APP_FFD_MAC_802_15_4_TraceError("ERROR : ERR_IPCC_SEND_ACK_TO_RFCore ",ErrCode);
    break;
  default :
    APP_FFD_MAC_802_15_4_TraceError("ERROR Unknown ",0);
    break;
  }
}


void APP_FFD_MAC_802_15_4_SendData(uint16_t dst_addr, DS2_Packet* data)
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
  DataReq.ack_Tx = TRUE;
  DataReq.GTS_Tx = FALSE;
  memcpy(&rfBuffer,(uint8_t*)data,data_len);
  rfBuffer[data_len] = xorSign((char*)data,data_len);
  DataReq.msduPtr = (uint8_t*) rfBuffer;
  DataReq.msdu_length = data_len;
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

void APP_FFD_MAC_802_15_4_SendEcho(void)
{
	char data[] = "COORDINATOR ECHO\0";

	  MAC_Status_t MacStatus = MAC_ERROR;

	  uint16_t g_panId = 0x1AAA;
	  uint16_t g_dst_addr = 0xFFFF;
	  static uint16_t g_dataHandle = 0x03;

	  BSP_LED_On(LED3);
	  MAC_dataReq_t DataReq;
	  //APP_DBG("FFD MAC APP - Send Data to NODE\0");
	  DataReq.src_addr_mode = g_SHORT_ADDR_MODE_c;
	  DataReq.dst_addr_mode = g_SHORT_ADDR_MODE_c;

	  memcpy(DataReq.a_dst_PAN_id,&g_panId,0x02);
	  memcpy(DataReq.dst_address.a_short_addr,&g_dst_addr,0x02);

	  DataReq.msdu_handle = g_dataHandle++;
	  DataReq.ack_Tx = TRUE;
	  DataReq.GTS_Tx = FALSE;
	  memcpy(&rfBuffer,data,strlen(data));
	  rfBuffer[strlen(data)] = xorSign(data,strlen(data));
	  DataReq.msduPtr = (uint8_t*) rfBuffer;
	  DataReq.msdu_length = strlen(data)+1;
	  DataReq.security_level = 0x00;
	  MacStatus = MAC_MCPSDataReq( &DataReq );
	  if ( MAC_SUCCESS != MacStatus ) {
	    APP_DBG("FFD MAC - Data Req Fails\0");
	    return;
	  }
	  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_DATA_DATA_CNF );
	  BSP_LED_Off(LED3);
	  //APP_DBG("FFD MAC APP - DATA CNF Received\0");
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void APP_FFD_MAC_802_15_4_Config()
{
  APP_DBG("configure FFD MAC 802.15.4 - 2");
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
  macCbConfig.mlmePollIndCb = APP_MAC_mlmePollIndCb;
}

/**
  * @brief  Warn the user that an error has occurred.In this case,
  *         the LEDs on the Board will start blinking.
  *
  * @param  pMess  : Message associated to the error.
  * @param  ErrCode: Error code associated to the module (OpenThread or other module if any)
  * @retval None
  */
static void APP_FFD_MAC_802_15_4_TraceError(char * pMess, uint32_t ErrCode)
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


/************** APP FFD ********************/

/* Private variables ---------------------------------------------------------*/
/** @defgroup APP FFD private variables
 * @{
 */

/**
 * @}
 */

/* Global variables ----------------------------------------------------------*/
/** @defgroup APP FFD Global variable
 * @{
 */
/**
 * @}
 */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup APP FFD private function prototypes
 * @{
 */

static void APP_FFD_MAC_802_15_4_DS2_Abort(void)
{
	APP_DBG("DS2 ABORT");

	memset(g_Parties, 0, sizeof(g_Parties));
	memset(g_packet_cnt, 0, sizeof(g_packet_cnt));

	g_AppState = DS2_IDLE;

    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
    g_msg_buffer.msg_code = DS2_ABORT;
    g_msg_buffer.packet_length = 4;

    for(int i = 0; i < 1; i++){
    	APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
    }
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;

	switch(g_AppState){
	case DS2_READY:
		APP_DBG("DS2 - KYEGEN STAGE 1");
		g_AppState = DS2_KEYGEN_STAGE_1_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
	case DS2_KEYGEN_STAGE_1_IDLE:

		memcpy(g_Parties[src_id].pi_commit, (uint8_t*)packet_ptr->data, DS2_Pi_COMMIT_SIZE);
		g_Parties[src_id].status |= DS2_Pi_COMMIT_FLAG;

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Pi_COMMIT_FLAG);
		}

		if(ready_flag)
		{
		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = 0xff;
		    g_msg_buffer.msg_code = DS2_Pi_COMMIT_ACK;
		    g_msg_buffer.packet_length = 4;

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		    g_AppState = DS2_KEYGEN_STAGE_1_END;
		}
		break;
	default:
		APP_DBG("DS2 - ERROR: KYEGEN STAGE 1 TASK TRIGGERED FORM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_1_END:
		APP_DBG("DS2 - KYEGEN STAGE 2");
		g_AppState = DS2_KEYGEN_STAGE_2_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
	case DS2_KEYGEN_STAGE_2_IDLE:

		memcpy(g_Parties[src_id].pi_val, (uint8_t*)packet_ptr->data, DS2_Pi_VALUE_SIZE);
		g_Parties[src_id].status |= DS2_Pi_VALUE_FLAG ;

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Pi_VALUE_FLAG);
		}

		if(ready_flag)
		{
			//rho = H(rho_i)

		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = 0xff;
		    g_msg_buffer.msg_code = DS2_Pi_VALUE_ACK;
		    g_msg_buffer.packet_length = DS2_HEADER_LEN + SEED_BYTES;
		    g_msg_buffer.data_offset = 0;
		    memcpy(g_msg_buffer.data, g_rho, SEED_BYTES);

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		    g_AppState = DS2_KEYGEN_STAGE_2_END;
		}
		break;
	default:
		APP_DBG("DS2 - ERROR: KYEGEN STAGE 2 TASK TRIGGERED FORM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_2_END:
		APP_DBG("DS2 - KYEGEN STAGE 3");
		g_AppState = DS2_KEYGEN_STAGE_3_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
	case DS2_KEYGEN_STAGE_3_IDLE:

		memcpy(g_Parties[src_id].ti_commit, (uint8_t*)packet_ptr->data, DS2_Ti_COMMIT_SIZE);
		g_Parties[src_id].status |= DS2_Ti_COMMIT_FLAG ;

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Ti_COMMIT_FLAG);
		}

		if(ready_flag)
		{
		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = 0xff;
		    g_msg_buffer.msg_code = DS2_Ti_COMMIT_ACK;
		    g_msg_buffer.packet_length = 4;

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		    g_AppState = DS2_KEYGEN_STAGE_3_END;
		}
		break;
	default:
		APP_DBG("DS2 - ERROR: KYEGEN STAGE 3 TASK TRIGGERED FORM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Final(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_3_END:
		APP_DBG("DS2 - KYEGEN FINAL");
		g_AppState = DS2_KEYGEN_FINAL_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
	case DS2_KEYGEN_FINAL_IDLE:

		g_packet_cnt[src_id]++;
		memcpy((g_Parties[src_id].ti_val+offset), (uint8_t*)packet_ptr->data, data_size);

		//all packets from node src_id were received
		if(g_packet_cnt[src_id]*DS2_MAX_DATA_LEN*4 > DS2_Ti_VALUE_SIZE){
			g_Parties[src_id].status |= DS2_Pi_VALUE_FLAG ;
		}

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Pi_VALUE_FLAG);
		}

		if(ready_flag)
		{
			//t1 = sum(ti)

		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = 0xff;
		    g_msg_buffer.msg_code = DS2_Ti_VALUE_ACK;
		    g_msg_buffer.packet_length = 4;
		    /*
		    g_msg_buffer.packet_length = DS2_HEADER_LEN + SEED_BYTES;
		    g_msg_buffer.data_offset = 0;
		    memcpy(g_msg_buffer.data, g_rho, SEED_BYTES);
		    */
		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		    g_AppState = DS2_KEYGEN_FINAL_END;
		}
		break;
	default:
		APP_DBG("DS2 - ERROR: KYEGEN FINAL TASK TRIGGERED FORM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_NewConnection(void)
{
	DS2_Packet *packet_ptr_rx = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint16_t dst_addr = 0;
	memcpy((uint8_t*)&dst_addr, g_DataInd_rx.src_address.a_short_addr, 0x2);
	uint8_t id = packet_ptr_rx->src_node_id;

	APP_DBG("DS2 - COORDINATOR HELLO - NODE ADDR:%d ID:%d", dst_addr, id);
	g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
	g_msg_buffer.dst_node_id = id;
	g_msg_buffer.packet_length = 4;

	if(id >= DS2_MAX_PARTY_NUM){
		g_msg_buffer.msg_code = DS2_ERROR_INVALID_NODE_ID;
	}
	else {
		if(g_Parties[id].status == 0){
			g_Parties[id].status |= DS2_PARTY_ACTIVE;
			g_msg_buffer.msg_code = DS2_COORDINATOR_HELLO_ACK;
		}
		else {
			g_msg_buffer.msg_code = DS2_ERROR_NODE_ID_ALREADY_IN_USE;
		}
	}

	//send ACK to node
	APP_FFD_MAC_802_15_4_SendData(dst_addr, &g_msg_buffer);

	// check if all nodes ready
	uint32_t ready_flag = 0xFFFFFFFF;
	for(int i = 1; i < sizeof(g_Parties); i++){
		ready_flag &= (g_Parties[id].status & DS2_PARTY_ACTIVE);
	}

	if(ready_flag){
		APP_DBG("DS2 - COORDINATOR READY !");
		g_AppState = DS2_READY;
		g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		g_msg_buffer.msg_code = DS2_COORDINATOR_READY_RESET,
		g_msg_buffer.packet_length = 4;
		//send READY
		APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	}
/*
	if(msg_buffer.msg_code > DS2_ABORT)
	{
		MAC_disassociateReq_t DisAssocReq = {0};
		MAC_Status_t MacStatus = MAC_ERROR;

		APP_DBG("DS2 - ERROR: NODE_ID_ALREADY_IN_USE");
		APP_DBG("FFD MAC APP - DISASSOCIATING ...");
		DisAssocReq.device_addr_mode = g_SHORT_ADDR_MODE_c;
		memcpy(DisAssocReq.a_device_PAN_id,&g_panId,0x02);
		memcpy(DisAssocReq.device_address.a_short_addr,(uint8_t)&dst_addr,0x02);
		DisAssocReq.disassociate_reason = msg_buffer.msg_code;
		DisAssocReq.tx_Indirect =0x00;
		DisAssocReq.security_level = 0x00;
		MacStatus = MAC_MLMEDisassociateReq( &DisAssocReq );
		if ( MAC_SUCCESS != MacStatus ) {
		    APP_DBG("FFD MAC APP - DISASSOCIATION ERROR");
		    return;
		}
		APP_DBG("FFD MAC APP - DISASSOCIATION - OK");
	}*/
}

static uint8_t xorSign( const char * pmessage, uint8_t message_len)
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
/** @defgroup APP FFD Private function
 * @{
 */
/**
 * @}
 */

/* Functions Definition ------------------------------------------------------*/
/** @defgroup APP FFD function prototypes
 * @{
 */


/**
 * @}
 */


