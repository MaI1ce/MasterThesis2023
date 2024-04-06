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

/////////////////////////////////////////
#include "commit.h"
#include "elapsed_time.h"
#include "fips202.h"
#include "poly.h"
#include "usbd_cdc_if.h"
////////////////////////////////////////


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

#define TIMER_SIGN_COMMIT		13

#define TIMER_PACKET_SEND		14

MAC_associateInd_t g_MAC_associateInd;

extern UART_HandleTypeDef huart1;

typedef struct {
	uint32_t length;
	uint8_t msg_code;
	uint8_t node_id;
}serial_frame;

#define FRAME_HEADER_SIZE 6


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

static void APP_FFD_MAC_802_15_4_DS2_Sign_Start(void);
static void APP_FFD_MAC_802_15_4_DS2_Sign_Stage_1(void);
static void APP_FFD_MAC_802_15_4_DS2_Sign_Stage_2(void);
static void APP_FFD_MAC_802_15_4_DS2_Sign_Stage_3(void);
static void APP_FFD_MAC_802_15_4_DS2_Sign_Final(void);

void UART_TxCpltCallback(void);
static void APP_FFD_MAC_802_15_4_DS2_UART_RX_CMD(void);
static void APP_FFD_MAC_802_15_4_DS2_UART_RX_DATA(void);

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Reset(void);
static void APP_FFD_MAC_802_15_4_DS2_Sign_Reset(void);

void APP_FFD_MAC_802_15_4_SendEcho(void);

static uint8_t xorSign( const char * pmessage, uint32_t message_len);

//extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
//extern uint32_t UserRxBufferFS_len;

static uint8_t 		UART1_rxBuffer[DS2_Fi_COMMIT_SIZE+FRAME_HEADER_SIZE];
static uint8_t		UART1_CMD_DATA_Flag = 0; // 0 - CMD;  1 - DATA
static uint32_t 	UART1_rxBuffer_Len = 0;
static uint8_t 		UART1_txBuffer[DS2_Fi_COMMIT_SIZE+FRAME_HEADER_SIZE];
static serial_frame* tx_ptr = (serial_frame*)UART1_txBuffer;
static serial_frame* rx_ptr = (serial_frame*)UART1_rxBuffer;

static uint8_t 		g_ERROR_code = 0;
static DS2_Party 	g_Parties[DS2_MAX_PARTY_NUM] = {0};
static uint16_t 	g_packet_cnt[DS2_MAX_PARTY_NUM] = {0};
static DS2_Packet 	g_msg_buffer = {0};


static uint8_t 		g_rho[SEED_BYTES] = {0};
static poly_t		t1[K] = {0};
static poly_t		A[K][L] = {0};
static uint8_t		tr[SEED_BYTES] = {0};

static uint32_t 	iterations = 0;
static poly_t		Commit[K][K] = {0};
static uint8_t		c[SEED_BYTES] = {0};
static uint8_t 		ck_seed[SEED_BYTES] = {0};
static poly_t		poly_c = {0};
static poly_t		z1[L] = {0};
static poly_t		z2[K] = {0};

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
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_RESET, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Reset);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_1, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_2, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_STAGE_3, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_KEYGEN_FINAL, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_KeyGen_Final);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_START, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_Sign_Start);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_STAGE_1, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_Sign_Stage_1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_STAGE_2, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_Sign_Stage_2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_STAGE_3, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_Sign_Stage_3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_SIGN_FINAL, UTIL_SEQ_RFU,APP_FFD_MAC_802_15_4_DS2_Sign_Final);

  UTIL_SEQ_RegTask( 1<<CFG_TASK_DS2_USB_TX, UTIL_SEQ_RFU, APP_FFD_MAC_802_15_4_DS2_UART_RX_DATA);
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


	tx_ptr->msg_code = DS2_COORDINATOR_READY_RESET;
	tx_ptr->node_id = 254;
	tx_ptr->length = 2;
	UART1_CMD_DATA_Flag = 0;
	HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);

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
  elapsed_time_start(TIMER_PACKET_SEND);
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
    APP_DBG("FFD MAC - Data Req Fails\0");
    return;
  }
  UTIL_SEQ_WaitEvt( 1U << CFG_EVT_DATA_DATA_CNF );
  BSP_LED_Off(LED3);
  //APP_DBG("RFD MAC APP - DATA CNF Received\0");
  elapsed_time_stop(TIMER_PACKET_SEND);
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
	  DataReq.ack_Tx = FALSE;
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
static void APP_FFD_MAC_802_15_4_DS2_UART_RX_CMD(void){
	uint32_t data_len = rx_ptr->length;
	uint8_t msg_code = rx_ptr->msg_code;
	uint8_t node_id = rx_ptr->node_id;

	if (data_len > 2){
		HW_UART_Receive_DMA(CFG_CLI_UART, UART1_rxBuffer+FRAME_HEADER_SIZE, data_len-2, APP_FFD_MAC_802_15_4_DS2_UART_RX_DATA);
		return;
	}

	if(msg_code >= DS2_ABORT){
		g_ERROR_code = msg_code;
		APP_FFD_MAC_802_15_4_DS2_Abort();
		return;
	}

	switch(msg_code){
	case DS2_KEYGEN_START_TASK:
		APP_DBG("FFD DS2 - UART RX CMD CALLBACK - START KEYGEN");
		break;
	default:
		APP_DBG("FFD DS2 - UART RX CMD CALLBACK - UNKNOWN COMMAND");
		return;
	}


	g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
	g_msg_buffer.dst_node_id = DS2_BROADCAST_ID; //node_id
	g_msg_buffer.msg_code = msg_code;
	g_msg_buffer.packet_length = 4;
	APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

	HW_UART_Receive_DMA(CFG_CLI_UART, UART1_rxBuffer, FRAME_HEADER_SIZE, APP_FFD_MAC_802_15_4_DS2_UART_RX_CMD);
}

static void APP_FFD_MAC_802_15_4_DS2_UART_RX_DATA(void)
{

	uint32_t data_len = rx_ptr->length;
	uint8_t msg_code = rx_ptr->msg_code;
	//uint8_t node_id = rx_ptr->node_id;
	uint8_t *data = UART1_rxBuffer+FRAME_HEADER_SIZE;

	if(msg_code >= DS2_ABORT){
		g_ERROR_code = msg_code;
		APP_FFD_MAC_802_15_4_DS2_Abort();
		return;
	}

	APP_DBG("FFD DS2 - UART RX DATA CALLBACK - MSG CODE %d LEN %d", msg_code, data_len);

	switch(msg_code){
	case DS2_SIGN_START_TASK:
		APP_DBG("FFD DS2 - UART RX CMD CALLBACK - START SIGN");
		break;
	case DS2_Pi_VALUE_ACK:
		break;
	case DS2_Ti_VALUE_ACK:
		UART1_CMD_DATA_Flag = 0;
		break;
	case DS2_Fi_COMMIT_ACK:
		break;
	case DS2_Zi_2_VALUE_ACK:
		UART1_CMD_DATA_Flag = 0;
		break;
	default:
		APP_DBG("FFD DS2 - UART RX DATA CALLBACK - UNKNOWN COMMAND");
		return;
	}

	memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));
	int i = 0;

	g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
	g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
	g_msg_buffer.msg_code = msg_code;
	g_msg_buffer.packet_length = DS2_HEADER_LEN + (DS2_MAX_DATA_LEN * 4);
	g_msg_buffer.data_offset = 0;
	uint8_t packet_num = data_len / (DS2_MAX_DATA_LEN * 4);
	uint8_t last_data_len = data_len % (DS2_MAX_DATA_LEN * 4);

	for(i = 0; i < packet_num; i++){
		g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);

		memcpy((char*)g_msg_buffer.data, &data[g_msg_buffer.data_offset], (DS2_MAX_DATA_LEN * 4));

		APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	}

	if(last_data_len > 0){
		g_msg_buffer.data_offset = i * (DS2_MAX_DATA_LEN * 4);
		g_msg_buffer.packet_length = last_data_len + DS2_HEADER_LEN;

		memcpy((char*)g_msg_buffer.data, &data[g_msg_buffer.data_offset], last_data_len);

		APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	}

	HW_UART_Receive_DMA(CFG_CLI_UART, UART1_rxBuffer, FRAME_HEADER_SIZE, APP_FFD_MAC_802_15_4_DS2_UART_RX_CMD);
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Reset(void)
{
	 APP_FFD_MAC_802_15_4_DS2_Sign_Reset();

	memset((char*)&g_msg_buffer, 0, sizeof(g_msg_buffer));
	memset(g_Parties, 0, sizeof(g_Parties));
	memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
	memset(t1, 0, sizeof(t1));
	memset(tr, 0, sizeof(tr));
	memset(g_rho, 0 , sizeof(g_rho));

	g_AppState = DS2_READY;
}

static void APP_FFD_MAC_802_15_4_DS2_Sign_Reset(void)
{
	memset(Commit, 0, sizeof(Commit));
	memset(c, 0, sizeof(c));
	memset(&poly_c, 0, sizeof(poly_c));
	memset(ck_seed, 0, sizeof(ck_seed));
	memset(z1, 0, sizeof(z1));
	memset(z2, 0 , sizeof(z2));

	for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
		g_Parties[i].status &= (~DS2_Fi_COMMIT_FLAG);
		g_Parties[i].status &= (~DS2_Ri_VALUE_FLAG);
		g_Parties[i].status &= (~DS2_Zi_1_VALUE_FLAG);
		g_Parties[i].status &= (~DS2_Zi_2_VALUE_FLAG);
	}

	elapsed_time_clr(TIMER_PACKET_SEND);
	g_AppState = DS2_READY;
}

static void APP_FFD_MAC_802_15_4_DS2_Abort(void)
{
	UART1_CMD_DATA_Flag = 0;
	UART_TxCpltCallback();
	switch(g_ERROR_code){
	case DS2_ABORT:
		APP_DBG("FFD FFD DS2 ERROR CODE : %d", g_ERROR_code);
		break;
	case DS2_ERROR_INVALID_NODE_ID:
		APP_DBG("FFD DS2 ERROR CODE : %d - DS2_ERROR_INVALID_NODE_ID", g_ERROR_code);
		break;
	case DS2_ERROR_NODE_ID_ALREADY_IN_USE:
		APP_DBG("FFD DS2 ERROR CODE : %d - DS2_ERROR_NODE_ID_ALREADY_IN_USE", g_ERROR_code);
		break;
	case DS2_ERROR_Ti_COMMIT:
		APP_DBG("FFD DS2 ERROR CODE : %d - DS2_ERROR_Ti_COMMIT", g_ERROR_code);
		break;
	case DS2_ERROR_Pi_COMMIT:
		APP_DBG("DS2 ERROR CODE : %d - DS2_ERROR_Pi_COMMIT", g_ERROR_code);
		break;
	case DS2_ERROR_Fi_COMMIT:
		APP_DBG("FFD DS2 ERROR CODE : %d - DS2_ERROR_Fi_COMMIT", g_ERROR_code);
		break;
	case DS2_ERROR_Zi_REJECT:
		APP_FFD_MAC_802_15_4_DS2_Sign_Reset();
		APP_DBG("FFD DS2 ERROR CODE : %d - DS2_ERROR_Zi_REJECT", g_ERROR_code);
		return;
	}

    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
    g_msg_buffer.msg_code = g_ERROR_code;
    g_msg_buffer.packet_length = 4;

    for(int i = 0; i < 1; i++){
    	APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
    }

	APP_FFD_MAC_802_15_4_DS2_KeyGen_Reset();
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_1(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;

	switch(g_AppState){
	case DS2_READY:
		g_AppState = DS2_KEYGEN_STAGE_1_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
		elapsed_time_start(TIMER_KEYGEN_TOTAL);
#ifdef DS2_DEBUG
			APP_DBG("FFD DS2 - KEYGEN - STAGE 1");
#endif
	case DS2_KEYGEN_STAGE_1_IDLE:

		memcpy(g_Parties[src_id].pi_commit, packet_ptr->data, DS2_Pi_COMMIT_SIZE);
		g_Parties[src_id].status |= DS2_Pi_COMMIT_FLAG;

		///////////////////////
		memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, packet_ptr->data, DS2_Pi_COMMIT_SIZE);

		tx_ptr->msg_code = DS2_Pi_COMMIT;
		tx_ptr->node_id = src_id;
		tx_ptr->length = DS2_Pi_COMMIT_SIZE + 2;

#ifdef DS2_DEBUG

	        uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
	        APP_DBG("FFD DS2 - pi_commit[%d] = %ld", src_id, gix);
#endif

		HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);
		///////////////////////

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Pi_COMMIT_FLAG);
		}

		if(ready_flag)
		{


		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		    g_msg_buffer.msg_code = DS2_Pi_COMMIT_ACK;
		    g_msg_buffer.packet_length = 4;

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		    g_AppState = DS2_KEYGEN_STAGE_1_END;
		}
		break;
	default:
		APP_DBG("FFD DS2 - KYEGEN - ERROR: STAGE 1 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_2(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;

	//static uint8_t temp_commit[DS2_Pi_COMMIT_SIZE] = {0};

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_1_END:
		g_AppState = DS2_KEYGEN_STAGE_2_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 - KEYGEN - STAGE 2");
#endif
	case DS2_KEYGEN_STAGE_2_IDLE:
		//save pi value
		memcpy(g_Parties[src_id].pi_val, packet_ptr->data, DS2_Pi_VALUE_SIZE);

		g_Parties[src_id].status |= DS2_Pi_VALUE_FLAG ;

		///////////////////////
		memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, packet_ptr->data, DS2_Pi_VALUE_SIZE);

		tx_ptr->msg_code = DS2_Pi_VALUE;
		tx_ptr->node_id = src_id;
		tx_ptr->length = DS2_Pi_VALUE_SIZE + 2;

		UART1_CMD_DATA_Flag = 1;
		UART1_rxBuffer_Len = SEED_BYTES + FRAME_HEADER_SIZE; //expect rho in response

#ifdef DS2_DEBUG
			uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
	        APP_DBG("FFD DS2 - pi_val[%d] = %ld", src_id, gix);
#endif

		HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);
		///////////////////////

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Pi_VALUE_FLAG);
		}

		if(ready_flag)
		{
/*
			elapsed_time_start(TIMER_KEYGEN_STAGE_2);
			//compute commitment
			h1(g_Parties[src_id].pi_val, src_id, temp_commit);

			// check if commitment is correct
			if(memcmp(temp_commit, g_Parties[src_id].pi_commit, DS2_Pi_COMMIT_SIZE) != 0){
				elapsed_time_stop(TIMER_KEYGEN_STAGE_2);
				g_ERROR_code = DS2_ERROR_Pi_COMMIT;
				UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_ABORT, CFG_SCH_PRIO_0 );
				return;
			}
			//get_rho
			//rho = H(pi)
			keccak_state_t state;
			keccak_init(&state);
			for(int i = 0; i < DS2_MAX_PARTY_NUM; i++)
			{
				shake128_absorb(&state, g_Parties[i].pi_val, SEED_BYTES);
			}
			shake128_finalize(&state);
			shake128_squeeze(&state, SEED_BYTES, g_rho);

			poly_uniform(g_rho, K * L, 0, &A[0][0]);

			elapsed_time_stop(TIMER_KEYGEN_STAGE_2);

#ifdef DS2_DEBUG
        uint8_t pix = xorSign((char*)g_Parties[src_id].pi_val, sizeof(g_Parties[src_id].pi_val));
        APP_DBG("FFD DS2 - KEYGEN - pi = %ld", pix);

        uint8_t gix = xorSign((char*)temp_commit, sizeof(temp_commit));
        APP_DBG("FFD DS2 - KEYGEN - gi* = h1(pi) = %ld", gix);

		uint8_t rhox =  xorSign((char*)g_rho, sizeof(g_rho));
		APP_DBG("FFD DS2 - KEYGEN - rho = h(p0, p1, ... pi) = %ld", rhox);

		APP_DBG("FFD DS2 - KEYGEN - A = H(rho) = %ld", A[1][1].coeffs[_N-1]);
#endif

			//send rho
		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		    g_msg_buffer.msg_code = DS2_Pi_VALUE_ACK;
		    g_msg_buffer.packet_length = DS2_HEADER_LEN + SEED_BYTES;
		    g_msg_buffer.data_offset = 0;

		    memcpy(g_msg_buffer.data, g_rho, SEED_BYTES);

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
*/
		    g_AppState = DS2_KEYGEN_STAGE_2_END;
		}
		break;
	default:
		APP_DBG("FFD DS2 - KYEGEN - ERROR: STAGE 2 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Stage_3(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;

	//uint8_t usb_buffer[DS2_Ti_COMMIT_SIZE+3] = {0};

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_2_END:
		g_AppState = DS2_KEYGEN_STAGE_3_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 - KEYGEN - STAGE 3");
#endif
	case DS2_KEYGEN_STAGE_3_IDLE:

		memcpy(g_Parties[src_id].ti_commit, (uint8_t*)packet_ptr->data, DS2_Ti_COMMIT_SIZE);
		g_Parties[src_id].status |= DS2_Ti_COMMIT_FLAG ;

		///////////////////////
		memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, packet_ptr->data, DS2_Ti_COMMIT_SIZE);

		tx_ptr->msg_code = DS2_Ti_COMMIT;
		tx_ptr->node_id = src_id;
		tx_ptr->length = DS2_Ti_COMMIT_SIZE + 2;

#ifdef DS2_DEBUG
			uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
	        APP_DBG("FFD DS2 - ti_commit[%d] = %ld", src_id, gix);
#endif

		HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, NULL);
		///////////////////////

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Ti_COMMIT_FLAG);
		}

		if(ready_flag)
		{
		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		    g_msg_buffer.msg_code = DS2_Ti_COMMIT_ACK;
		    g_msg_buffer.packet_length = 4;

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		    g_AppState = DS2_KEYGEN_STAGE_3_END;
		}
		break;
	default:
		APP_DBG("FFD DS2 - KYEGEN - ERROR: STAGE 3 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_KeyGen_Final(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	//uint8_t usb_buffer[DS2_Ti_VALUE_SIZE+3] = {0};

	//poly_t temp_ti[K] = {0};
	//uint8_t t1_packed[DS2_Ti_VALUE_SIZE] = {0};

	//uint8_t temp_commit[DS2_Ti_COMMIT_SIZE] = {0};

	switch(g_AppState){
	case DS2_KEYGEN_STAGE_3_END:
		g_AppState = DS2_KEYGEN_FINAL_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 - KEYGEN - STAGE 4");
#endif
	case DS2_KEYGEN_FINAL_IDLE:

		g_packet_cnt[src_id]++;
		memcpy(&g_Parties[src_id].ti_val[offset], (uint8_t*)packet_ptr->data, data_size);
		APP_DBG("FFD DS2 - ID: %d PACKET: %d SIZE: %d OFFSET: %d",src_id, g_packet_cnt[src_id], data_size, offset);

		//all packets from node src_id were received
		if(g_packet_cnt[src_id]*DS2_MAX_DATA_LEN*4 > DS2_Ti_VALUE_SIZE){
			g_Parties[src_id].status |= DS2_Ti_VALUE_FLAG ;


			memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, g_Parties[src_id].ti_val, DS2_Ti_VALUE_SIZE);

			tx_ptr->msg_code = DS2_Ti_VALUE;
			tx_ptr->node_id = src_id;
			tx_ptr->length = DS2_Ti_VALUE_SIZE + 2;
			UART1_CMD_DATA_Flag = 1;
			UART1_rxBuffer_Len = SEED_BYTES + FRAME_HEADER_SIZE; //expect tr in response

			APP_DBG("FFD DS2 - SEND UART");

#ifdef DS2_DEBUG
			//uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
			uint8_t gix = UART1_txBuffer[tx_ptr->length+3];
	        APP_DBG("FFD DS2 - ti_val[%d] = %ld", src_id, gix);
#endif

			HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);
		}

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Ti_VALUE_FLAG);
		}

		if(ready_flag)
		{
/*
			elapsed_time_start(TIMER_KEYGEN_FINAL);

			h2(g_Parties[src_id].ti_val, src_id, temp_commit);

			//check commit ti
			if(memcmp(temp_commit, g_Parties[src_id].ti_commit, DS2_Ti_COMMIT_SIZE) != 0){
				g_ERROR_code = DS2_ERROR_Ti_COMMIT;
				UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_ABORT, CFG_SCH_PRIO_0 );
				APP_DBG("FFD DS2 - KEYGEN - ABORT");
				return;
			}

			//get_t1
			//t1 = sum(ti)
			for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
				poly_unpack(T1_BITS, g_Parties[i].ti_val, K, 0, temp_ti);
				poly_add(t1, temp_ti, K, t1);
			}
			poly_freeze(t1, K);

			//generate tr
		    keccak_state_t state;
		    keccak_init(&state);
		    shake128_absorb(&state, g_rho, SEED_BYTES);
		    shake128_absorb(&state, (uint8_t*)t1, sizeof(t1));
		    shake128_finalize(&state);
		    shake128_squeeze(&state, SEED_BYTES, tr);

		    //send t1
		    uint8_t packet_num = DS2_Ti_VALUE_SIZE / (DS2_MAX_DATA_LEN * 4);
		    uint8_t last_data_len = DS2_Ti_VALUE_SIZE % (DS2_MAX_DATA_LEN * 4);

		    //poly_pack(T1_BITS, t1, K, t1_packed);

		    elapsed_time_stop(TIMER_KEYGEN_FINAL);

#ifdef DS2_DEBUG
			uint8_t gtix =  xorSign((char*)temp_commit, sizeof(temp_commit));
			APP_DBG("FFD DS2 - KEYGEN - gti* = h2(ti) = %ld", gtix);

			APP_DBG("RFD DS2 - KEYGEN - t1 = sum(t0, t1, ... ti) = %ld", t1[1].coeffs[_N-1]);

		    uint8_t trx =  xorSign((char*)tr, sizeof(tr));
		    APP_DBG("FFD DS2 - KEYGEN - tr = h(rho, t1) = %ld", trx);
#endif

		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		    g_msg_buffer.msg_code = DS2_Ti_VALUE_ACK;
		    g_msg_buffer.packet_length = DS2_HEADER_LEN + sizeof(tr);
		    g_msg_buffer.data_offset = 0;
		    memcpy(g_msg_buffer.data, &tr, sizeof(tr));

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
*/
		    g_AppState = DS2_READY;

		    elapsed_time_stop(TIMER_KEYGEN_TOTAL);

		    APP_DBG("FFD DS2 - KYEGEN - STAGE 0 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_START));
		    APP_DBG("FFD DS2 - KYEGEN - STAGE 1 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_STAGE_1));
		    APP_DBG("FFD DS2 - KYEGEN - STAGE 2 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_STAGE_2));
		    APP_DBG("FFD DS2 - KYEGEN - STAGE 3 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_STAGE_3));
		    APP_DBG("FFD DS2 - KYEGEN - STAGE 4 TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_FINAL));
		    APP_DBG("FFD DS2 - KYEGEN - TOTAL   TIMER:%ld",elapsed_time_max(TIMER_KEYGEN_TOTAL));
		    APP_DBG("FFD DS2 - KYEGEN - PACKET  TIMER:%ld",elapsed_time_max(TIMER_PACKET_SEND));
		}
		break;
	default:
		APP_DBG("FFD DS2 - KYEGEN - ERROR: STAGE 4 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

//////////////////////////////////////////////////
static void APP_FFD_MAC_802_15_4_DS2_Sign_Start(void)
{
	APP_FFD_MAC_802_15_4_DS2_Sign_Reset();
	g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
	g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
	g_msg_buffer.msg_code = DS2_SIGN_START_TASK;
	g_msg_buffer.packet_length = 4;
	APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
}


static void APP_FFD_MAC_802_15_4_DS2_Sign_Stage_1(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	poly_t fi[K][K] = {0};
	uint8_t f_packed[DS2_Fi_COMMIT_SIZE] = {0};

	static char msg[] = "test message\0";
	static uint32_t msg_len = sizeof(msg);


	switch(g_AppState){
	case DS2_READY:
		elapsed_time_start(TIMER_SIGN_TOTAL);
		g_AppState = DS2_SIGN_STAGE_1_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 1");
#endif

	case DS2_SIGN_STAGE_1_IDLE:

		g_packet_cnt[src_id]++;
		memcpy(&g_Parties[src_id].fi_commit[offset], (uint8_t*)packet_ptr->data, data_size);
		//APP_DBG("DS2 - ID: %d PACKET: %d SIZE: %d OFFSET: %d",src_id, g_packet_cnt[src_id], data_size, offset);

		//all packets from node src_id were received
		if(g_packet_cnt[src_id]*DS2_MAX_DATA_LEN*4 > DS2_Fi_COMMIT_SIZE){
			g_Parties[src_id].status |= DS2_Fi_COMMIT_FLAG ;


			memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, g_Parties[src_id].fi_commit, DS2_Fi_COMMIT_SIZE);

			tx_ptr->msg_code = DS2_Fi_COMMIT;
			tx_ptr->node_id = src_id;
			tx_ptr->length = DS2_Fi_COMMIT_SIZE + 2;
			UART1_rxBuffer_Len = SEED_BYTES + FRAME_HEADER_SIZE; //expect sc in response

			APP_DBG("FFD DS2 - SEND FI UART");

#ifdef DS2_DEBUG
			//uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
			uint8_t gix = UART1_txBuffer[tx_ptr->length+3];
	        APP_DBG("FFD DS2 - Fi_val[%d] = %ld", src_id, gix);
#endif

			HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);
		}

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Fi_COMMIT_FLAG);
		}

		if(ready_flag)
		{
			/*
			elapsed_time_start(TIMER_SIGN_STAGE_1);
			//commit = sum(f_i)
			for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
				poly_unpack(TC_L, g_Parties[i].fi_commit, K*K, 0, (poly_t*)fi);
				poly_add((poly_t*)Commit, (poly_t*)fi, K*K, (poly_t*)Commit);
#ifdef DS2_DEBUG
			    APP_DBG("FFD DS2 -- SIGN -- com_[%d] = ck*ri + wi = %ld",i, fi[1][1].coeffs[_N-1]);
#endif
			}
			poly_freeze((poly_t*)Commit, K*K);

			poly_pack(TC_L, (poly_t*)Commit, K*K, f_packed);

	        //c = H(com, msg, pk)
		    keccak_state_t state;
	        keccak_init(&state);
		    shake256_absorb(&state, msg, msg_len);
		    shake256_absorb(&state, tr, SEED_BYTES);
		    shake256_finalize(&state);
		    shake256_squeeze(&state, SEED_BYTES, ck_seed);

		    keccak_init(&state);
		    shake256_absorb(&state, f_packed, sizeof(f_packed));
		    shake256_absorb(&state, ck_seed, sizeof(ck_seed));
		    shake256_finalize(&state);
		    shake256_squeeze(&state, SEED_BYTES, c);

	        poly_challenge(c, &poly_c);
	        poly_ntt(&poly_c, 1);

	        elapsed_time_stop(TIMER_SIGN_STAGE_1);

#ifdef DS2_DEBUG
		    uint8_t ckx =  xorSign((char*)ck_seed, sizeof(ck_seed));
		    APP_DBG("FFD DS2 -- SIGN -- ck_seed = h3(tr, msg) = %ld", ckx);

		    uint8_t cx =  xorSign((char*)c, sizeof(c));
		    APP_DBG("FFD DS2 -- SIGN -- c = H(com, msg, pk) = %ld", cx);
#endif

		    g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		    g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		    g_msg_buffer.msg_code = DS2_Fi_COMMIT_ACK;
		    g_msg_buffer.data_offset = 0;
		    g_msg_buffer.packet_length = DS2_HEADER_LEN + sizeof(c);

		    memcpy(g_msg_buffer.data, c, sizeof(c));

		    APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
*/
			g_AppState = DS2_SIGN_STAGE_1_END;

		}
		break;
	default:
		APP_DBG("FFD DS2 -- SIGN -- ERROR: STAGE 1 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_Sign_Stage_2(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	switch(g_AppState){
	case DS2_SIGN_STAGE_1_END:

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 2");
#endif
		g_AppState = DS2_SIGN_STAGE_2_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));
	case DS2_SIGN_STAGE_2_IDLE:

		memcpy(&g_Parties[src_id].ri_val, (uint8_t*)packet_ptr->data, data_size);
		//all packets from node src_id were received
		g_Parties[src_id].status |= DS2_Ri_VALUE_FLAG;

		memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, g_Parties[src_id].ri_val, DS2_Ri_VALUE_SIZE);

		tx_ptr->msg_code = DS2_Ri_VALUE;
		tx_ptr->node_id = src_id;
		tx_ptr->length = DS2_Ri_VALUE_SIZE + 2;

#ifdef DS2_DEBUG
		uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
		APP_DBG("FFD DS2 - Ri_val[%d] = %ld", src_id, gix);
#endif

		HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);


		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Ri_VALUE_FLAG);
		}

		if(ready_flag)
		{

			g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
			g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
			g_msg_buffer.msg_code = DS2_Ri_VALUE_ACK;
			g_msg_buffer.packet_length = 4;
			APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

			g_AppState = DS2_SIGN_STAGE_2_END;

		}
		break;
	default:
		APP_DBG("FFD DS2 -- SIGN -- ERROR: STAGE 2 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_Sign_Stage_3(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	switch(g_AppState){
	case DS2_SIGN_STAGE_2_END:
		g_AppState = DS2_SIGN_STAGE_3_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 3");
#endif

	case DS2_SIGN_STAGE_3_IDLE:


		g_packet_cnt[src_id]++;
		memcpy(&g_Parties[src_id].zi_1_val[offset], (uint8_t*)packet_ptr->data, data_size);
		//APP_DBG("DS2 - ID: %d PACKET: %d SIZE: %d OFFSET: %d",src_id, g_packet_cnt[src_id], data_size, offset);

		//all packets from node src_id were received
		if(g_packet_cnt[src_id]*DS2_MAX_DATA_LEN*4 > DS2_Zi_1_VALUE_SIZE){
			g_Parties[src_id].status |= DS2_Zi_1_VALUE_FLAG ;

			memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, g_Parties[src_id].zi_1_val, DS2_Zi_1_VALUE_SIZE);

			tx_ptr->msg_code = DS2_Zi_1_VALUE;
			tx_ptr->node_id = src_id;
			tx_ptr->length = DS2_Zi_1_VALUE_SIZE + 2;

	#ifdef DS2_DEBUG
			//uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
			uint8_t gix = UART1_txBuffer[3+tx_ptr->length];
			APP_DBG("FFD DS2 - Zi_1_val[%d] = %ld", src_id, gix);
	#endif

			HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);
		}

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Zi_1_VALUE_FLAG);
		}

		if(ready_flag)
		{

			g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
			g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
			g_msg_buffer.msg_code = DS2_Zi_1_VALUE_ACK;
			g_msg_buffer.packet_length = 4;
			APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

		g_AppState = DS2_SIGN_STAGE_3_END;

		}
		break;
	default:
		APP_DBG("FFD DS2 -- SIGN -- ERROR: STAGE 3 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

static void APP_FFD_MAC_802_15_4_DS2_Sign_Final(void)
{
	DS2_Packet *packet_ptr = (DS2_Packet *)g_DataInd_rx.msduPtr;
	uint8_t src_id = packet_ptr->src_node_id;
	uint32_t offset = packet_ptr->data_offset;
	uint8_t data_size = packet_ptr->packet_length - DS2_HEADER_LEN;

	poly_t z1_temp[L] = {0};
	poly_t z2_temp[K] = {0};
	poly_t t1_temp[K] = {0};
	poly_t w_temp[K] = {0};
	poly_t F1[K][K] = {0};
	poly_t F2[K][K] = {0};

	switch(g_AppState){
	case DS2_SIGN_STAGE_3_END:
		g_AppState = DS2_SIGN_FINAL_IDLE;
		memset(g_packet_cnt, 0, sizeof(g_packet_cnt));

#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- STAGE 4");
#endif
	case DS2_SIGN_FINAL_IDLE:

		g_packet_cnt[src_id]++;
		memcpy(&g_Parties[src_id].zi_2_val[offset], (uint8_t*)packet_ptr->data, data_size);
		//APP_DBG("DS2 - ID: %d PACKET: %d SIZE: %d OFFSET: %d",src_id, g_packet_cnt[src_id], data_size, offset);

		//all packets from node src_id were received
		if(g_packet_cnt[src_id]*DS2_MAX_DATA_LEN*4 > DS2_Zi_2_VALUE_SIZE){
			g_Parties[src_id].status |= DS2_Zi_2_VALUE_FLAG ;

			memcpy(UART1_txBuffer + FRAME_HEADER_SIZE, g_Parties[src_id].zi_1_val, DS2_Zi_2_VALUE_SIZE);

			tx_ptr->msg_code = DS2_Zi_2_VALUE;
			tx_ptr->node_id = src_id;
			tx_ptr->length = DS2_Zi_2_VALUE_SIZE + 2;

	#ifdef DS2_DEBUG
			//uint8_t gix = xorSign((char*)UART1_txBuffer, 4+tx_ptr->length);
			uint8_t gix = UART1_txBuffer[3+tx_ptr->length];
			APP_DBG("FFD DS2 - Zi_2_val[%d] = %ld", src_id, gix);
	#endif

			HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);
		}

		uint32_t ready_flag = 0xFFFFFFFF;
		for(int i = 0; i < DS2_MAX_PARTY_NUM; i++){
			ready_flag &= (g_Parties[i].status & DS2_Zi_2_VALUE_FLAG);
		}

		if(ready_flag)
		{

			/*
			elapsed_time_start(TIMER_SIGN_FINAL);

			uint8_t rej = 0;
			for(int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
	            poly_unpack(TC_L, g_Parties[i].zi_1_val, L, 0, z1_temp);
	            poly_unpack(TC_L, g_Parties[i].zi_2_val, K, 0, z2_temp);

#ifdef DS2_DEBUG
			    uint8_t rx =  xorSign((char*)g_Parties[i].ri_val, sizeof(g_Parties[i].ri_val));
			    APP_DBG("FFD DS2 -- SIGN -- r[%d]  = %ld", i, rx);
	            APP_DBG("FFD DS2 -- SIGN -- z1[%d] = %ld", i, z1_temp[1].coeffs[_N-1]);
	            APP_DBG("FFD DS2 -- SIGN -- z2[%d] = %ld", i, z2_temp[1].coeffs[_N-1]);
#endif

	            poly_center(z1_temp, L);
	            poly_center(z2_temp, K);

	            poly_add(z1, z1_temp, L, z1);
	            poly_add(z2, z2_temp, K, z2);

	            // w = Az1 - ct1 * 2^D
	            poly_unpack(T1_BITS, g_Parties[i].ti_val, K, 0, t1_temp);

	            // t1 * 2^D
	            poly_const_mul(t1_temp, 1 << D, K, t1_temp);

	            poly_ntt(z1_temp, L);
	            poly_ntt(t1_temp, K);

	            poly_product(A, z1_temp, w_temp);
	            poly_mul_pointwise(t1_temp, &poly_c, K, t1_temp);

	            poly_reduce(w_temp, K);
	            poly_reduce(t1_temp, K);

	            poly_invntt_tomont(w_temp, K);
	            poly_invntt_tomont(t1_temp, K);

	            poly_add(w_temp, z2_temp, K, w_temp);
	            poly_sub(w_temp, t1_temp, K, w_temp);

	            poly_freeze(w_temp, K);

	    		// com_i = ck*r + w
	            poly_unpack(TC_L, g_Parties[i].fi_commit, K*K, 0, (poly_t*)F2);

#ifdef DS2_DEBUG
	            APP_DBG("FFD DS2 -- SIGN -- OPEN COMMITMENT com_i ...");
#endif
	            elapsed_time_start(TIMER_SIGN_COMMIT);
	    		poly_gen_commit(ck_seed, g_Parties[i].ri_val, (poly_t*)F1);
	    		poly_add((poly_t*)&F1[1], (poly_t*)w_temp, K, (poly_t*)&F1[1]);

	    		poly_freeze((poly_t*)F1, K*K);
	    		elapsed_time_stop(TIMER_SIGN_COMMIT);

#ifdef DS2_DEBUG
	            APP_DBG("FFD DS2 -- SIGN -- w[%ld] = Az_i - ct0_i*2^D = %ld", i, w_temp[1].coeffs[_N-1]);
	            APP_DBG("FFD DS2 -- SIGN -- com_i[%ld] = ck*r_i + w_i = %ld", i, F1[1][1].coeffs[_N-1]);
	            APP_DBG("FFD DS2 -- SIGN -- open(ck, r_i, w_i, com_i) = %ld", F2[1][1].coeffs[_N-1]);
#endif
	    		//check commitments
	    		rej = memcmp(F1, F2, sizeof(F1));

	    		if(rej != 0){
	    			elapsed_time_stop(TIMER_SIGN_FINAL);

	    			APP_DBG("FFD DS2 -- SIGN -- STAGE 4 REJECTED");

	    			g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
	    			g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
	    			g_msg_buffer.msg_code = DS2_ERROR_Fi_COMMIT;
	    			g_msg_buffer.packet_length = 4;
	    			APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);
	    			return;
	    		}
			}
			elapsed_time_stop(TIMER_SIGN_FINAL);
*/
			g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
			g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
			g_msg_buffer.msg_code = DS2_Zi_2_VALUE_ACK;
			g_msg_buffer.packet_length = 4;
			APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);

			g_AppState = DS2_READY;

			elapsed_time_stop(TIMER_SIGN_TOTAL);
			APP_DBG("FFD DS2 -- SIGN -- STAGE 0 TIMER:%ld",elapsed_time_max(TIMER_SIGN_START));
			APP_DBG("FFD DS2 -- SIGN -- STAGE 1 TIMER:%ld",elapsed_time_max(TIMER_SIGN_STAGE_1));
			APP_DBG("FFD DS2 -- SIGN -- STAGE 2 TIMER:%ld",elapsed_time_max(TIMER_SIGN_STAGE_2));
			APP_DBG("FFD DS2 -- SIGN -- STAGE 3 TIMER:%ld",elapsed_time_max(TIMER_SIGN_STAGE_3));
			APP_DBG("FFD DS2 -- SIGN -- STAGE 4 TIMER:%ld",elapsed_time_max(TIMER_SIGN_FINAL));
			APP_DBG("FFD DS2 -- SIGN -- TOTAL   TIMER:%ld",elapsed_time_max(TIMER_SIGN_TOTAL));
			APP_DBG("FFD DS2 -- SIGN -- COMMIT  TIMER:%ld",elapsed_time_max(TIMER_SIGN_COMMIT));
			APP_DBG("FFD DS2 -- SIGN -- PACKET  TIMER:%ld",elapsed_time_max(TIMER_PACKET_SEND));
		}
		break;
	default:
		APP_DBG("RFD DS2 -- SIGN -- ERROR: STAGE 4 TASK TRIGGERED FROM BAD STATE %d", g_AppState);
		break;
	}
}

//////////////////////////////////////////////////

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

	if(ready_flag && (g_msg_buffer.msg_code < DS2_ABORT)){
		APP_DBG("DS2 - COORDINATOR READY !");
		g_AppState = DS2_READY;
		g_msg_buffer.src_node_id = DS2_COORDINATOR_ID;
		g_msg_buffer.dst_node_id = DS2_BROADCAST_ID;
		g_msg_buffer.msg_code = DS2_COORDINATOR_READY_RESET,
		g_msg_buffer.packet_length = 4;
		//send READY
		APP_FFD_MAC_802_15_4_SendData(0xFFFF, &g_msg_buffer);


		tx_ptr->msg_code = DS2_COORDINATOR_HELLO;
		tx_ptr->node_id = id;
		tx_ptr->length = 2;
		UART1_CMD_DATA_Flag = 0;
		HW_UART_Transmit_DMA(CFG_CLI_UART, UART1_txBuffer, 4+tx_ptr->length, UART_TxCpltCallback);

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

static uint8_t xorSign( const char * pmessage, uint32_t message_len)
{
  uint8_t seed = 0x00;
  for (uint8_t i=0x00;i<message_len;i++)
    seed = (uint8_t)pmessage[i]^seed;
  return seed;
}



void UART_TxCpltCallback(void) {
	HW_UART_Receive_DMA(CFG_CLI_UART, UART1_rxBuffer, FRAME_HEADER_SIZE,  APP_FFD_MAC_802_15_4_DS2_UART_RX_CMD);
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


