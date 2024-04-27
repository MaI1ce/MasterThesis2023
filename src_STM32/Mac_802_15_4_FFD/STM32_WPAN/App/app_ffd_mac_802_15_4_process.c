/**
  ******************************************************************************
* @file    app_mac_802-15-4_process.c
* @author  MDG-RF Team
* @brief   MAC Cli Implementation for Demo Purpose
*
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

#include "app_ffd_mac_802_15_4_process.h"

#include "app_conf.h"
#include "dbg_trace.h"
#include "shci.h"
#include "stm32_seq.h"
#include "app_ffd_mac_802_15_4.h"
#include "802_15_4_mac_sap.h"
#include "stm_logging.h"

/* Global define -------------------------------------------------------------*/
#ifndef FALSE
#define FALSE false
#endif
#ifndef TRUE
#define TRUE true
#endif
/* Global variables ----------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/
#define MAX_PIB_ATTRIBUTE_VALUE_LENGTH 52

#define DATA "HELLO NODE\0"

/* Private macros ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/

uint8_t checkMsgXorSignature(const char * pMessage, uint8_t message_len, 
                             uint8_t sign, uint8_t expectedRes);
/* Public variables ----------------------------------------------------------*/

extern MAC_associateInd_t g_MAC_associateInd;
MAC_dataInd_t      g_DataInd_rx;


/**
* Init
*/

/******************************************************************************/


/******************************************************************************/


/* callbacks to be called by the MAC
* Create an NTS message and enqueue it in the IPCC queue
*/

MAC_Status_t APP_MAC_mlmeAssociateCnfCb( const  MAC_associateCnf_t * pAssociateCnf )
{
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeAssociateIndCb( const  MAC_associateInd_t * pAssociateInd )
{
  /* Store ASSOCIATION IND */
  MAC_Status_t MacStatus = MAC_ERROR;

  MAC_associateRes_t AssociateRes;

  APP_DBG("FFD MAC APP - Association Requested - APP_MAC_mlmeAssociateIndCb");

  static uint16_t shortAssociationAddr = 0x3300;

  memcpy(&g_MAC_associateInd,pAssociateInd,sizeof(MAC_associateInd_t));

  memcpy(AssociateRes.a_device_address,g_MAC_associateInd.a_device_address,0x08);
  memcpy(AssociateRes.a_assoc_short_address,&shortAssociationAddr,0x02);
  AssociateRes.security_level = 0x00;
  AssociateRes.status = MAC_SUCCESS;

  MacStatus = MAC_MLMEAssociateRes(&AssociateRes);
  if ( MAC_SUCCESS != MacStatus ) {
   APP_DBG("FFD MAC - Association Response Fails - APP_MAC_mlmeAssociateIndCb");
  }
  HAL_Delay(200);
  BSP_LED_Off(LED2);
  shortAssociationAddr++;
  return MAC_SUCCESS;
}

MAC_Status_t APP_MAC_mlmeBeaconNotifyIndCb( const  MAC_beaconNotifyInd_t * pBeaconNotifyInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeBeaconNotifyIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeCommStatusIndCb( const  MAC_commStatusInd_t * pCommStatusInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeCommStatusIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeDisassociateCnfCb( const  MAC_disassociateCnf_t * pDisassociateCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeDisassociateCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeDisassociateIndCb( const  MAC_disassociateInd_t * pDisassociateInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeDisassociateIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mlmeGetCnfCb( const  MAC_getCnf_t * pGetCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeGetCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mlmeOrphanIndCb( const  MAC_orphanInd_t * pOrphanInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeOrphanIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mlmePollCnfCb( const  MAC_pollCnf_t * pPollCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmePollCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mlmeResetCnfCb( const  MAC_resetCnf_t * pResetCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeResetCnfCb");
  UTIL_SEQ_SetEvt(EVENT_DEVICE_RESET_CNF);
  return MAC_SUCCESS;
}


MAC_Status_t APP_MAC_mlmeRxEnableCnfCb( const  MAC_rxEnableCnf_t * pRxEnableCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeRxEnableCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mlmeScanCnfCb( const  MAC_scanCnf_t * pScanCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeScanCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeSetCnfCb( const  MAC_setCnf_t * pSetCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeSetCnfCb");
  UTIL_SEQ_SetEvt(EVENT_SET_CNF);
  return MAC_SUCCESS;
}


MAC_Status_t APP_MAC_mlmeStartCnfCb( const  MAC_startCnf_t * pStartCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeStartCnfCb");
  UTIL_SEQ_SetEvt(EVENT_DEVICE_STARTED_CNF);
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mcpsDataIndCb( const  MAC_dataInd_t * pDataInd )
{

  memcpy(&g_DataInd_rx,pDataInd,sizeof(MAC_dataInd_t));
  DS2_Packet *packet_ptr = (DS2_Packet *)pDataInd->msduPtr;
  // Check validity of the received Message extracting associated 
  // simple xor signature
  //APP_DBG("SRC:%d DST:%d MSG_CODE:%d", packet_ptr->src_node_id, packet_ptr->dst_node_id, packet_ptr->msg_code);
  if (!checkMsgXorSignature((char const *)(pDataInd->msduPtr),
                           pDataInd->msdu_length-1,
                           pDataInd->msduPtr[pDataInd->msdu_length-1],
                           0x00))
  {
    APP_DBG("FFD MAC APP - ERROR : CORRUPTED RECEIVED DATA %d", pDataInd->msdu_length);
  }
  else
  {
		//APP_DBG("COORD : RECEIVE DATA : %s ", (char const *) pDataInd->msduPtr);
	  if (packet_ptr != NULL){
			if(packet_ptr->packet_length < 4){
				APP_DBG("DS2 DATA ERROR - MSG IS TOO SHORT");
			}
			else {
				if((packet_ptr->dst_node_id == DS2_BROADCAST_ID)||(packet_ptr->dst_node_id == DS2_COORDINATOR_ID)){
					HW_UART_Transmit_DMA(CFG_CLI_UART, (uint8_t*)pDataInd->msduPtr, pDataInd->msdu_length-1, UART_TxCpltCallback);

					switch(packet_ptr->msg_code){
					case DS2_KEYGEN_START_TASK:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_RESET, CFG_SCH_PRIO_0 );
						break;
					case DS2_COORDINATOR_HELLO:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_NEW_CONNECTION, CFG_SCH_PRIO_0 );
						break;
					case DS2_SIGN_START_TASK: case DS2_ERROR_Zi_REJECT:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_RESET_SIGN, CFG_SCH_PRIO_0 );
						break;/*
					case DS2_Pi_COMMIT:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_KEYGEN_STAGE_1, CFG_SCH_PRIO_0 );
						break;
					case DS2_Pi_VALUE:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_KEYGEN_STAGE_2, CFG_SCH_PRIO_0 );
						break;
					case DS2_Ti_COMMIT:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_KEYGEN_STAGE_3, CFG_SCH_PRIO_0 );
						break;
					case DS2_Ti_VALUE:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_KEYGEN_FINAL, CFG_SCH_PRIO_0 );
						break;

					case DS2_Fi_COMMIT:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_SIGN_STAGE_1, CFG_SCH_PRIO_0 );
						break;

					case DS2_Ri_VALUE:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_SIGN_STAGE_2, CFG_SCH_PRIO_0 );
						break;

					case DS2_Zi_1_VALUE:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_SIGN_STAGE_3, CFG_SCH_PRIO_0 );
						break;

					case DS2_Zi_2_VALUE:
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_SIGN_FINAL, CFG_SCH_PRIO_0 );
						break;

					case DS2_VERIFY_START_TASK:
						break;

					default :
						//APP_DBG("DS2 DATA ERROR - JOB ABORTED");
						UTIL_SEQ_SetTask( 1<< CFG_TASK_DS2_ABORT, CFG_SCH_PRIO_0 );
						break;*/
					}
				} else {
					APP_DBG("DS2 DATA ERROR - WRONG DST NODE ID %d", packet_ptr->dst_node_id);
				}

			}
		}
  }
  return MAC_SUCCESS;
}

MAC_Status_t APP_MAC_mcpsDataCnfCb( const  MAC_dataCnf_t * pDataCnf )
{
  //APP_DBG("FFD MAC - APP_MAC_mcpsDataCnfCb");
  UTIL_SEQ_SetEvt( EVENT_DATA_CNF );
  return MAC_SUCCESS;
}

MAC_Status_t APP_MAC_mcpsPurgeCnfCb( const  MAC_purgeCnf_t * pPurgeCnf )
{
	//APP_DBG("FFD MAC - APP_MAC_mcpsPurgeCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;

}

MAC_Status_t APP_MAC_mlmeSyncLossIndCb( const MAC_syncLoss_t * syncLossPtr )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeSyncLossIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeCalibrateCnfCb( const MAC_calibrateCnf_t * pCallibrateCnf)
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeCalibrateCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}



MAC_Status_t APP_MAC_mlmeDpsCnfCb( const MAC_dpsCnf_t * pDpsCnf  )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeDpsCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeDpsIndCb( const MAC_dpsInd_t * pDpsInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeDpsIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeSoundingCnfCb( const MAC_soundingCnf_t * pSoudingCnf)
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeSoundingCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeGtsCnfCb( const MAC_gtsCnf_t * pGtsCnf)
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeGtsCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeGtsIndCb( const MAC_GtsInd_t * pGtsInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmeGtsIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

// APP_MAC_mlmePollIndCbPtr  mlmePollIndCb;
MAC_Status_t APP_MAC_mlmePollIndCb( const MAC_pollInd_t * pPollInd )
{
	//APP_DBG("FFD MAC - APP_MAC_mlmePollIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

/* Private function Definition -----------------------------------------------*/

uint8_t checkMsgXorSignature(const char * pMessage, uint8_t message_len, 
                             uint8_t sign, uint8_t expectedRes)
{
  uint8_t seed = sign;
  for (uint8_t i=0x00;i<message_len;i++)
    seed = (uint8_t)pMessage[i]^seed;
  return (seed==expectedRes);
}

/******************************************************************************/

//@} \\END CLI Commands

