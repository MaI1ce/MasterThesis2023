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

#define DEMO_CHANNEL 20

#define DATA_FROM_NODE "DATA FROM NODE\0"
#define DATA "HELLO COORDINATOR\0"

/* Private function prototypes -----------------------------------------------*/


uint8_t xorSign( const char * pmessage, uint8_t message_len);


static void APP_RFD_MAC_802_15_4_TraceError(char * pMess, uint32_t ErrCode);
static void APP_RFD_MAC_802_15_4_Config(void);

////////////////////////////////////////////////////////////////////////
static void APP_RFD_MAC_802_15_4_DS2_Abort(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Start(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void);
static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Final(void);


DS2_Packet g_msg_buffer = {0};
////////////////////////////////////////////////////////////////////////
static uint8_t rfBuffer[256];

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

  UTIL_SEQ_RegTask( 1<<CFG_TASK_APP_ABORT, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_Abort);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_APP_KEYGEN_START, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Start);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_APP_KEYGEN_STAGE_1, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_APP_KEYGEN_STAGE_2, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_APP_KEYGEN_STAGE_3, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_APP_KEYGEN_FINAL, UTIL_SEQ_RFU, APP_RFD_MAC_802_15_4_DS2_KeyGen_Final);

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

uint8_t xorSign( const char * pmessage, uint8_t message_len)
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
	g_msg_buffer.src_node_id = DS2_NODE_ID;
	g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
	g_msg_buffer.msg_code = DS2_KEYGEN_START;
	g_msg_buffer.packet_length = 4;
	APP_RFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

	APP_RFD_MAC_802_15_4_DS2_KeyGen_Start();
}

static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Start(void)
{
	APP_DBG("DS2 - KYEGEN START");
	memset((char*)&g_msg_buffer, 1, sizeof(g_msg_buffer));

    g_msg_buffer.src_node_id = DS2_NODE_ID;
    g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
    g_msg_buffer.msg_code = DS2_Pi_COMMIT;
    g_msg_buffer.packet_length = DS2_HEADER_LEN + DS2_Pi_COMMIT_SIZE; //send g_i - size 64 byte
    g_msg_buffer.data_offset = 0;

    memset((char*)&g_msg_buffer.data, 1, DS2_Pi_COMMIT_SIZE);

    APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
}


static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void)
{
	APP_DBG("DS2 - KYEGEN STAGE 1");
	memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));

    g_msg_buffer.src_node_id = DS2_NODE_ID;
    g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
    g_msg_buffer.msg_code = DS2_Pi_VALUE;
    g_msg_buffer.packet_length = DS2_HEADER_LEN + DS2_Pi_VALUE_SIZE;

    memset((char*)&g_msg_buffer.data, 2, DS2_Pi_VALUE_SIZE);
    g_msg_buffer.data_offset = 0;

    APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
}


static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void)
{
	APP_DBG("DS2 - KYEGEN STAGE 2");
	memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));

    g_msg_buffer.src_node_id = DS2_NODE_ID;
    g_msg_buffer.dst_node_id = DS2_COORDINATOR_ID;
    g_msg_buffer.msg_code = DS2_Ti_COMMIT;
    g_msg_buffer.packet_length = DS2_HEADER_LEN + DS2_Ti_COMMIT_SIZE;

    memset((char*)&g_msg_buffer.data, 3, DS2_Ti_COMMIT_SIZE);
    g_msg_buffer.data_offset = 0;

    APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
}


static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void)
{
	APP_DBG("DS2 - KYEGEN STAGE 3");
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

    	memset((char*)g_msg_buffer.data, i, (DS2_MAX_DATA_LEN * 4));

    	APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
    }

    if(last_data_len > 0){
    	g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);
    	g_msg_buffer.packet_length = last_data_len + DS2_HEADER_LEN;

    	memset((char*)g_msg_buffer.data, i, last_data_len);

    	APP_RFD_MAC_802_15_4_SendData(g_coordShortAddr, &g_msg_buffer);
    }
}

static void APP_RFD_MAC_802_15_4_DS2_KeyGen_Final(void)
{
	APP_DBG("DS2 - KYEGEN FINAL");

}

static void APP_RFD_MAC_802_15_4_DS2_Abort(void)
{
	APP_DBG("DS2 DATA ERROR - JOB ABORTED");
	APP_DBG("DS2 ERROR CODE : %d", g_msg_buffer.msg_code);

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


