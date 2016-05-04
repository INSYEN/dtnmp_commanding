/******************************************************************************
 **                           COPYRIGHT NOTICE
 **      (c) 2012 The Johns Hopkins University Applied Physics Laboratory
 **                         All rights reserved.
 **
 **     This material may only be used, modified, or reproduced by or for the
 **       U.S. Government pursuant to the license rights granted under
 **          FAR clause 52.227-14 or DFARS clauses 252.227-7013/7014
 **
 **     For any other permissions, please contact the Legal Office at JHU/APL.
 ******************************************************************************/

/*****************************************************************************
 **
 ** File Name: adm_cgr_priv.c
 **
 ** Description: This implements the private aspects of a CGR ADM.
 **
 ** Notes:
 **
 ** Assumptions:
 **
 **
 ** Modification History:
 **  MM/DD/YY  AUTHOR         DESCRIPTION
 **  --------  ------------   ---------------------------------------------
 **  03/02/15  J.P. Mayer     Initial Implementation
 *****************************************************************************/
#include "ion.h"
#include "lyst.h"
#include "platform.h"

//For contact and plan stuff
#include "ipnfw.h"
#include "rfx.h"
#include "shared/utils/utils.h"

#include "adm_cgr_priv.h"
const uint16_t cgr_dlist_response_ion = 1;

void agent_adm_init_cgr()
{
	/* Node-specific Information. */
	uint8_t mid_str[ADM_MID_ALLOC];
	adm_build_mid_str(0, CGR_ADM_DATA_NN, CGR_ADM_DATA_NN_LEN, 0, mid_str);
	adm_add_datadef_collect(mid_str, cgr_node_get_contacts);

	adm_build_mid_str(0, CGR_ADM_DATA_NN, CGR_ADM_DATA_NN_LEN, 1, mid_str);
	adm_add_datadef_collect(mid_str, cgr_node_get_ranges);


	/* Controls */
	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 0, mid_str);
	adm_add_ctrl_run(mid_str,  cgr_ctrl_contact_add);

	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 1, mid_str);
	adm_add_ctrl_run(mid_str,  cgr_ctrl_contact_remove);

	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 3, mid_str);
	adm_add_ctrl_run(mid_str,  cgr_ctrl_range_add);

	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 4, mid_str);
	adm_add_ctrl_run(mid_str,  cgr_ctrl_range_remove);

}

/**********************************
	* The standard datalist contact format is (currently):
	* fromNode (uvast)
	* toNode (uvast)
	* startTime (uint64)
	* endTime (uint64)
	* probablility (real32)
	* xmitRate (uint32)
	*/

uint32_t cgr_ctrl_contact_add(Lyst params)
{
	LystElt elt = 0;
	uvast fromNode;
	uvast toNode;
	time_t startTime;
	time_t endTime;
	uint32_t xmitRate;
	uint32_t datacolSize;
	struct timeval	done_time;
	struct timeval	cur_time;

	DTNMP_DEBUG_INFO("cgr_ctrl_contact_add","()",NULL);
	//Sanity check
	if(lyst_length(params)<1) //Not enough params
	{
		return 0; //Check for proper return codes
	}
	//Parameter handling
	if (ionAttach() < 0)
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_add","ipnadmin can't attach to BP", NULL);
		return -1;
	}
	if(rfx_system_is_started()==0)
	{
		DTNMP_DEBUG_INFO("cgr_ctrl_contact_add","Starting RFX, please wait...",NULL);
		if (rfx_start() < 0)
		{
			DTNMP_DEBUG_ERR("cgr_ctrl_contact_add","Can't start RFX.", NULL);
			return 0;
		}
	}


	// Wait for rfx to start up.
	getCurrentTime(&done_time);
	done_time.tv_sec += STARTUP_TIMEOUT;
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);

	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		DTNMP_DEBUG_INFO("cgr_ctrl_contact_add","In loop",NULL);
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("cgr_ctrl_contact_add","Deserialized %d bytes to datalist",datacolSize);
		CHKVALID(datalist_get(&curDl,0,&fromNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&toNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,2,&startTime,NULL,DLIST_TYPE_UINT64));
		CHKVALID(datalist_get(&curDl,3,&endTime,NULL,DLIST_TYPE_UINT64));
		CHKVALID(datalist_get(&curDl,4,&xmitRate,NULL,DLIST_TYPE_UINT32));

        DTNMP_DEBUG_INFO("cgr_ctrl_contact","xmit %u",xmitRate);
		DTNMP_DEBUG_INFO("cgr_ctrl_contact_add","Trying to add f: %d t: %d (%ud %ud xmitRate: %u)",fromNode,toNode,startTime,endTime,xmitRate);
		//lets give it a shot
		if(rfx_insert_contact(startTime, endTime,fromNode,toNode, xmitRate,1.0)==0)
		{
			DTNMP_DEBUG_ERR("cgr_ctrl_contact_add","Couldn't add contact",NULL);
		}
		datalist_free_contents(&curDl);
	}
	utils_datacol_destroy(&dlDatacol);

	//_forecastNeeded(1);
	return 0;
}

uint32_t cgr_ctrl_contact_remove(Lyst params)
{
	LystElt elt = 0;
	unsigned short lystIdx = 0;
	uvast fromNode;
	uvast toNode;
	time_t startTime;
	uint32_t datacolSize;

	DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","()",NULL);
	//Sanity check
	if(lyst_length(params)<1) //Not enough params
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","No datalist found", NULL);
		return 0; //Check for proper return codes
	}
	//Parameter handling
	if (ionAttach() < 0)
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","can't attach to BP", NULL);
		return -1;
	}
	if (rfx_start() < 0)
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","Can't start RFX.", NULL);
	}
	struct timeval	done_time;
	struct timeval	cur_time;
	//Wait for rfx to start
	getCurrentTime(&done_time);
	done_time.tv_sec += STARTUP_TIMEOUT;
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);
	DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","Total number of datalists: %d",lyst_length(dlDatacol));
	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","Deserialized %d bytes to datalist",datacolSize);

		CHKVALID(datalist_get(&curDl,0,&fromNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&toNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,2,&startTime,NULL,DLIST_TYPE_UINT64));

		DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","Trying to remove %lu -> %lu",fromNode,toNode);

		if(rfx_remove_contact(startTime,fromNode,toNode)==0)
		{
			DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","Couldn't remove contact",NULL);
		}
		datalist_free_contents(&curDl);
	}
	//_forecastNeeded(1);
	utils_datacol_destroy(&dlDatacol);
	return 0;
}

uint32_t cgr_ctrl_range_add(Lyst params)
{
	LystElt elt = 0;
	uvast fromNode;
	uvast toNode;
	time_t startTime;
	time_t endTime;
	uint32_t range;
	uint32_t datacolSize;
	struct timeval	done_time;
	struct timeval	cur_time;

	DTNMP_DEBUG_INFO("cgr_ctrl_range_add","()",NULL);
	//Sanity check
	if(lyst_length(params)<1) //Not enough params
	{
		return 0; //Check for proper return codes
	}
	//Parameter handling
	if (ionAttach() < 0)
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_range_add","ipnadmin can't attach to BP", NULL);
		return -1;
	}
	if(rfx_system_is_started()==0)
	{
		DTNMP_DEBUG_INFO("cgr_ctrl_range_add","Starting RFX, please wait...",NULL);
		if (rfx_start() < 0)
		{
			DTNMP_DEBUG_ERR("cgr_ctrl_range_add","Can't start RFX.", NULL);
			return 0;
		}
	}


	// Wait for rfx to start up.
	getCurrentTime(&done_time);
	done_time.tv_sec += STARTUP_TIMEOUT;
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);

	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		DTNMP_DEBUG_INFO("cgr_ctrl_range_add","In loop",NULL);
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("cgr_ctrl_range_add","Deserialized %d bytes to datalist",datacolSize);
		CHKVALID(datalist_get(&curDl,0,&fromNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&toNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,2,&startTime,NULL,DLIST_TYPE_UINT64));
		CHKVALID(datalist_get(&curDl,3,&endTime,NULL,DLIST_TYPE_UINT64));
		CHKVALID(datalist_get(&curDl,4,&range,NULL,DLIST_TYPE_UINT32));

		DTNMP_DEBUG_INFO("cgr_ctrl_range_add","Trying to add f:%d t:%d %ud %ud %ud",fromNode,toNode,startTime,endTime,range);
		//Fuck it, lets give it a shot
		if(rfx_insert_range(startTime, endTime,fromNode,toNode, range)==0)
		{
			DTNMP_DEBUG_ERR("cgr_ctrl_range_add","Couldn't add range",NULL);
		}
		datalist_free_contents(&curDl);
	}
	utils_datacol_destroy(&dlDatacol);

	//_forecastNeeded(1);
	return 0;
}

uint32_t cgr_ctrl_range_remove(Lyst params)
{
	LystElt elt = 0;
	unsigned short lystIdx = 0;
	uvast fromNode;
	uvast toNode;
	time_t startTime;
	uint32_t datacolSize;

	DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","()",NULL);
	//Sanity check
	if(lyst_length(params)<1) //Not enough params
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","No datalist found", NULL);
		return 0; //Check for proper return codes
	}
	//Parameter handling
	if (ionAttach() < 0)
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","can't attach to BP", NULL);
		return -1;
	}
	if (rfx_start() < 0)
	{
		DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","Can't start RFX.", NULL);
	}
	struct timeval	done_time;
	struct timeval	cur_time;
	//Wait for rfx to start
	getCurrentTime(&done_time);
	done_time.tv_sec += STARTUP_TIMEOUT;
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);
	DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","Total number of datalists: %d",lyst_length(dlDatacol));
	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","Deserialized %d bytes to datalist",datacolSize);

		CHKVALID(datalist_get(&curDl,0,&fromNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&toNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,2,&startTime,NULL,DLIST_TYPE_UINT64));

		DTNMP_DEBUG_INFO("cgr_ctrl_contact_remove","Trying to remove %lu -> %lu",fromNode,toNode);

		if(rfx_remove_range(startTime,fromNode,toNode)==0)
		{
			DTNMP_DEBUG_ERR("cgr_ctrl_contact_remove","Couldn't remove contact",NULL);
		}
		datalist_free_contents(&curDl);
	}
	//_forecastNeeded(1);
	utils_datacol_destroy(&dlDatacol);
	return 0;
}

expr_result_t cgr_node_get_contacts(Lyst params)
{

	Sdr             sdr = getIonsdr();
	PsmPartition    ionwm = getIonwm();
	IonVdb          *vdb = getIonVdb();
	PsmAddress      elt;
	PsmAddress      addr;
	sdr_begin_xn(sdr);
	Lyst outdatacol = lyst_create();
	expr_result_t result;
	uint32_t resultLen; //Hacky hack hack.
	result.type=EXPR_TYPE_BLOB;
	for (elt = sm_rbt_first(ionwm, vdb->contactIndex); elt; elt = sm_rbt_next(ionwm, elt))
	{
		addr = sm_rbt_data(ionwm, elt);
		IonCXref        *contact;
        addr; //Put error handling here
        contact = (IonCXref *) psp(getIonwm(), addr);

        //Ok, now fill the datalist
        datalist_t contactDL=datalist_create(NULL);

		datalist_insert_with_type(&contactDL,DLIST_TYPE_UVAST,&contact->fromNode);
		datalist_insert_with_type(&contactDL,DLIST_TYPE_UVAST,&contact->toNode);
		datalist_insert_with_type(&contactDL,DLIST_TYPE_UINT64,&contact->fromTime);
		datalist_insert_with_type(&contactDL,DLIST_TYPE_UINT64,&contact->toTime);
		datalist_insert_with_type(&contactDL,DLIST_TYPE_REAL32,&contact->prob);
		datalist_insert_with_type(&contactDL,DLIST_TYPE_UINT32,&contact->xmitRate);

		DTNMP_DEBUG_INFO("adm_get_contacts","%d %d %u",contact->fromTime,contact->toTime,contact->xmitRate);
		datacol_entry_t* dlSerialized = datalist_serialize_to_datacol(&contactDL);

		lyst_insert_last(outdatacol,dlSerialized);
	}
	sdr_exit_xn(sdr);

	result.value=utils_datacol_serialize(outdatacol,&resultLen);
	result.length=(uint32_t)resultLen;

	return result;

}

expr_result_t cgr_node_get_ranges(Lyst params)
{
	Sdr             sdr = getIonsdr();
	PsmPartition    ionwm = getIonwm();
	IonVdb          *vdb = getIonVdb();
	PsmAddress      elt;
	PsmAddress      addr;
	sdr_begin_xn(sdr);
	Lyst outdatacol = lyst_create();
	expr_result_t result;
	uint32_t resultLen; //Hacky hack hack.
	result.type=EXPR_TYPE_BLOB;

	for (elt = sm_rbt_first(ionwm, vdb->rangeIndex); elt; elt = sm_rbt_next(ionwm, elt))
	{
		addr = sm_rbt_data(ionwm, elt);
		IonRXref        *range;
        addr; //Put error handling here
        range = (IonRXref *) psp(getIonwm(), addr);

        //Ok, now fill the datalist
        datalist_t rangeDL=datalist_create(NULL);

		datalist_insert_with_type(&rangeDL,DLIST_TYPE_UVAST,&range->fromNode);
		datalist_insert_with_type(&rangeDL,DLIST_TYPE_UVAST,&range->toNode);
		datalist_insert_with_type(&rangeDL,DLIST_TYPE_UVAST,&range->fromTime);
		datalist_insert_with_type(&rangeDL,DLIST_TYPE_UVAST,&range->toTime);
		datalist_insert_with_type(&rangeDL,DLIST_TYPE_UINT32,&range->owlt);

		datacol_entry_t* dlSerialized = datalist_serialize_to_datacol(&rangeDL);

		lyst_insert_last(outdatacol,dlSerialized);
	}
	sdr_exit_xn(sdr);

	result.value=utils_datacol_serialize(outdatacol,&resultLen);
	result.length=(uint32_t)resultLen;

	return result;

}

