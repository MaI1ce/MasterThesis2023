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

#include "app_rfd_mac_802_15_4_process.h"

#include "app_conf.h"
#include "dbg_trace.h"
#include "shci.h"
#include "stm32_seq.h"
#include "app_rfd_mac_802_15_4.h"
#include "stm_logging.h"

#include "ds2_osi3.h"
#include <string.h>

/* Global define -------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/


#ifndef FALSE
#define FALSE false
#endif
#ifndef TRUE
#define TRUE true
#endif


/* Private defines -----------------------------------------------------------*/
#define MAX_PIB_ATTRIBUTE_VALUE_LENGTH 52



/* Private macros ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/

/* Public variables ----------------------------------------------------------*/

extern MAC_associateCnf_t g_MAC_associateCnf;

/*
* Init
*/

/******************************************************************************/


/******************************************************************************/


/* callbacks to be called by the MAC
* Create an NTS message and enqueue it in the IPCC queue
*/

MAC_Status_t APP_MAC_mlmeAssociateCnfCb( const  MAC_associateCnf_t * pAssociateCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeAssociateCnfCb");
  APP_DBG("RFD MAC APP - Association CNF Received");
  memset(&g_MAC_associateCnf,0x00,sizeof(MAC_associateCnf_t));
  memcpy(&g_MAC_associateCnf,pAssociateCnf,sizeof(MAC_associateCnf_t));
  UTIL_SEQ_SetEvt(EVENT_SRV_ASSOC_CNF);
  return MAC_SUCCESS;

}

MAC_Status_t APP_MAC_mlmeAssociateIndCb( const  MAC_associateInd_t * pAssociateInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeAssociateIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeBeaconNotifyIndCb( const  MAC_beaconNotifyInd_t * pBeaconNotifyInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeBeaconNotifyIndCb");
  return MAC_SUCCESS;
}

MAC_Status_t APP_MAC_mlmeCommStatusIndCb( const  MAC_commStatusInd_t * pCommStatusInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeCommStatusIndCb");
  return MAC_SUCCESS;
}

MAC_Status_t APP_MAC_mlmeDisassociateCnfCb( const  MAC_disassociateCnf_t * pDisassociateCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeDisassociateCnfCb");
  return MAC_SUCCESS;
}

MAC_Status_t APP_MAC_mlmeDisassociateIndCb( const  MAC_disassociateInd_t * pDisassociateInd )
{
	APP_DBG("RFD MAC APP - Coordinator Disassociated");
  return MAC_SUCCESS;

}

MAC_Status_t APP_MAC_mlmeGetCnfCb( const  MAC_getCnf_t * pGetCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeGetCnfCb");
  return MAC_SUCCESS;

}

MAC_Status_t APP_MAC_mlmeOrphanIndCb( const  MAC_orphanInd_t * pOrphanInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeOrphanIndCb");
  return MAC_SUCCESS;

}

MAC_Status_t APP_MAC_mlmePollCnfCb( const  MAC_pollCnf_t * pPollCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmePollCnfCb");
  return MAC_SUCCESS;

}

MAC_Status_t APP_MAC_mlmeResetCnfCb( const  MAC_resetCnf_t * pResetCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeResetCnfCb");
  UTIL_SEQ_SetEvt(EVENT_DEVICE_RESET_CNF);
  return MAC_SUCCESS;
}


MAC_Status_t APP_MAC_mlmeRxEnableCnfCb( const  MAC_rxEnableCnf_t * pRxEnableCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeRxEnableCnfCb");
  return MAC_SUCCESS;

}

MAC_Status_t APP_MAC_mlmeScanCnfCb( const  MAC_scanCnf_t * pScanCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeScanCnfCb");
  return MAC_SUCCESS;
}


MAC_Status_t APP_MAC_mlmeSetCnfCb( const  MAC_setCnf_t * pSetCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeSetCnfCb");
  UTIL_SEQ_SetEvt(EVENT_SET_CNF);
  return MAC_SUCCESS;
}


MAC_Status_t APP_MAC_mlmeStartCnfCb( const  MAC_startCnf_t * pStartCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeStartCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mcpsDataIndCb( const  MAC_dataInd_t * pDataInd )
{
	DS2_packet *packet_ptr = (DS2_packet*)pDataInd->msduPtr;
  //APP_DBG("COORD : RECEIVE DATA : %s ", (char const *) pDataInd->msduPtr);
	if (packet_ptr != NULL){
		if(packet_ptr->packet_length < 4){
			APP_DBG("DS2 DATA ERROR - MSG IS TOO SHORT");
		}
		else {
			if((packet_ptr->dst_node_id == DS2_NODE_ID)||(packet_ptr->dst_node_id == DS2_BROADCAST_ID)){
				switch(packet_ptr->msg_code){
				case DS2_COORDINATOR_HELLO_ACK:
					APP_DBG("DS2 COORDINATOR CONNECTION - OK");
					break;
				case DS2_Pi_COMMIT_ACK:
					memcpy((char*)&msg_buffer, (char*)packet_ptr, packet_ptr->packet_length);
					UTIL_SEQ_SetTask( 1<< CFG_TASK_APP_KEYGEN_STAGE_1, CFG_SCH_PRIO_0 );
					break;
				case DS2_Pi_VALUE_ACK :
					memcpy((char*)&msg_buffer, (char*)packet_ptr, packet_ptr->packet_length);
					UTIL_SEQ_SetTask( 1<< CFG_TASK_APP_KEYGEN_STAGE_2, CFG_SCH_PRIO_0 );
					break;
				case DS2_Ti_COMMIT_ACK :
					memcpy((char*)&msg_buffer, (char*)packet_ptr, packet_ptr->packet_length);
					UTIL_SEQ_SetTask( 1<< CFG_TASK_APP_KEYGEN_STAGE_3, CFG_SCH_PRIO_0 );
					break;
				case DS2_Ti_VALUE_ACK :
					memcpy((char*)&msg_buffer, (char*)packet_ptr, packet_ptr->packet_length);
					UTIL_SEQ_SetTask( 1<< CFG_TASK_APP_KEYGEN_FINAL, CFG_SCH_PRIO_0 );
					break;

				case DS2_Fi_COMMIT_ACK:
					break;

				case DS2_Ri_VALUE_ACK:
					break;

				case DS2_Zi_VALUE_ACK:
					break;

				default :
					APP_DBG("DS2 DATA ERROR - JOB ABORTED");
					UTIL_SEQ_SetTask( 1<< CFG_TASK_APP_ABORT, CFG_SCH_PRIO_0 );
					break;
				}
			} else {
				APP_DBG("DS2 DATA ERROR - WRONG DST NODE ID");
			}
		}
	}
  return MAC_SUCCESS;
}


MAC_Status_t APP_MAC_mcpsDataCnfCb( const  MAC_dataCnf_t * pDataCnf )
{
	//APP_DBG("RFD MAC APP - APP_MAC_mcpsDataCnfCb");
  UTIL_SEQ_SetEvt( EVENT_DATA_CNF );
  return MAC_SUCCESS;

}


MAC_Status_t APP_MAC_mcpsPurgeCnfCb( const  MAC_purgeCnf_t * pPurgeCnf )
{
	APP_DBG("RFD MAC APP - APP_MAC_mcpsPurgeCnfCb");
  return MAC_SUCCESS;

}


MAC_Status_t APP_MAC_mlmeSyncLossIndCb( const MAC_syncLoss_t * syncLossPtr )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeSyncLossIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeCalibrateCnfCb( const MAC_calibrateCnf_t * pCallibrateCnf)
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeCalibrateCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}



MAC_Status_t APP_MAC_mlmeDpsCnfCb( const MAC_dpsCnf_t * pDpsCnf  )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeDpsCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeDpsIndCb( const MAC_dpsInd_t * pDpsInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeDpsIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeSoundingCnfCb( const MAC_soundingCnf_t * pSoudingCnf)
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeSoundingCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

MAC_Status_t APP_MAC_mlmeGtsCnfCb( const MAC_gtsCnf_t * pGtsCnf)
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeGtsCnfCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}


MAC_Status_t APP_MAC_mlmeGtsIndCb( const MAC_GtsInd_t * pGtsInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmeGtsIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}

// APP_MAC_mlmePollIndCbPtr  mlmePollIndCb;
MAC_Status_t APP_MAC_mlmePollIndCb( const MAC_pollInd_t * pPollInd )
{
	APP_DBG("RFD MAC APP - APP_MAC_mlmePollIndCb");
  return MAC_NOT_IMPLEMENTED_STATUS;
}
/******************************************************************************/

//@} \\END CLI Commands

