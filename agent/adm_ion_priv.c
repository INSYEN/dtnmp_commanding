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
 ** File Name: adm_ion_priv.c
 **
 ** Description: This implements the private aspects of an ION ADM.
 **
 ** Notes:
 **
 ** Assumptions:
 **
 **
 ** Modification History:
 **  MM/DD/YY  AUTHOR         DESCRIPTION
 **  --------  ------------   ---------------------------------------------
 **  07/16/13  E. Birrane     Initial Implementation
 *****************************************************************************/
#include "ion.h"
#include "platform.h"


#include "shared/adm/adm_ion.h"
#include "shared/utils/utils.h"
#include "bpP.h"
#include "ipnfw.h"
#include "rfx.h"
#include "adm_ion_priv.h"
#include "shared/primitives/datalist.h"
void agent_adm_init_ion()
{
	/* Register Nicknames */
	uint8_t mid_str[ADM_MID_ALLOC];

	/* ICI */
	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 0, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_sdr_state_all);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 1, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_small_pool_size);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 2, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_small_pool_free);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 3, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_small_pool_alloc);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 4, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_large_pool_size);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 5, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_large_pool_free);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 6, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_large_pool_alloc);

	adm_build_mid_str(0x00, ION_ADM_ICI_NN, ION_ADM_ICI_NN_LEN, 7, mid_str);
	adm_add_datadef_collect(mid_str,  ion_ici_get_unused_size);



	/* Inducts */
	/*
	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 0, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_all);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 1, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_name);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 2, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_last_reset);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 3, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_rx_bndl);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 4, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_rx_byte);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 5, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_mal_bndl);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 6, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_mal_byte);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 7, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_inauth_bndl);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 8, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_inauth_byte);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 9, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_over_bndl);

	adm_build_mid_str(0x40, ION_ADM_INDUCT_NN, ION_ADM_INDUCT_NN_LEN, 10, mid_str);
	adm_add_datadef_collect(mid_str,  ion_induct_get_over_byte);
*/

	/* Outducts */
	/*
	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 0, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_all);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 1, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_name);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 2, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_cur_q_bdnl);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 3, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_cur_q_byte);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 4, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_last_reset);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 5, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_enq_bndl);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 6, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_enq_byte);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 7, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_deq_bndl);

	adm_build_mid_str(0x40, ION_ADM_OUTDUCT_NN, ION_ADM_OUTDUCT_NN_LEN, 8, mid_str);
	adm_add_datadef_collect(mid_str,  ion_outduct_get_deq_byte);

*/

	/* Node */
/*
	adm_build_mid_str(0x00, ION_ADM_NODE_NN, ION_ADM_NODE_NN_LEN, 0, mid_str);
	adm_add_datadef_collect(mid_str,  ion_node_get_all);

	adm_build_mid_str(0x00, ION_ADM_NODE_NN, ION_ADM_NODE_NN_LEN, 1, mid_str);
	adm_add_datadef_collect(mid_str,  ion_node_get_inducts);

	adm_build_mid_str(0x00, ION_ADM_NODE_NN, ION_ADM_NODE_NN_LEN, 2, mid_str);
	adm_add_datadef_collect(mid_str,  ion_node_get_outducts);
*/



	adm_build_mid_str(0x00, ION_ADM_NODE_NN, ION_ADM_NODE_NN_LEN, 3, mid_str);
	adm_add_datadef_collect(mid_str,  ion_node_get_plans);

	adm_build_mid_str(0x00, ION_ADM_NODE_NN, ION_ADM_NODE_NN_LEN, 4, mid_str);
	adm_add_datadef_collect(mid_str,  ion_node_get_groups);

	/* Controls */
	adm_build_mid_str(0x01, ION_ADM_CTRL_NN, ION_ADM_CTRL_NN_LEN, 0, mid_str);
	adm_add_ctrl_run(mid_str,  ion_ctrl_induct_reset);

	adm_build_mid_str(0x01, ION_ADM_CTRL_NN, ION_ADM_CTRL_NN_LEN, 1, mid_str);
	adm_add_ctrl_run(mid_str,  ion_ctrl_outduct_reset);

	adm_build_mid_str(0x41, ION_ADM_CTRL_NN, ION_ADM_CTRL_NN_LEN, 2, mid_str);
	adm_add_ctrl_run(mid_str,  ion_ctrl_plan_add);

	adm_build_mid_str(0x41, ION_ADM_CTRL_NN, ION_ADM_CTRL_NN_LEN, 3, mid_str);
	adm_add_ctrl_run(mid_str,  ion_ctrl_plan_remove);

	adm_build_mid_str(0x41, ION_ADM_CTRL_NN, ION_ADM_CTRL_NN_LEN, 4, mid_str);
	adm_add_ctrl_run(mid_str,  ion_ctrl_group_add);

	adm_build_mid_str(0x41, ION_ADM_CTRL_NN, ION_ADM_CTRL_NN_LEN, 5, mid_str);
	adm_add_ctrl_run(mid_str,  ion_ctrl_group_remove);
}


/* Retrieval Functions. */

expr_result_t ion_ici_get_sdr_state_all(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_BLOB;

	result.length = sizeof(state);
	result.value = (uint8_t*) STAKE(result.length);
	sdrnm_state_get(&state);

	memcpy(result.value, &state, result.length);
	return result;
}

expr_result_t ion_ici_get_small_pool_size(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.smallPoolSize), sizeof(state.smallPoolSize), &(result.length));

	return result;
}

expr_result_t ion_ici_get_small_pool_free(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.smallPoolFree), sizeof(state.smallPoolFree), &(result.length));

	return result;
}

expr_result_t ion_ici_get_small_pool_alloc(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.smallPoolAllocated), sizeof(state.smallPoolAllocated), &(result.length));

	return result;
}

expr_result_t ion_ici_get_large_pool_size(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.largePoolSize), sizeof(state.largePoolSize), &(result.length));

	return result;
}

expr_result_t ion_ici_get_large_pool_free(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.largePoolFree), sizeof(state.largePoolFree), &(result.length));

	return result;
}

expr_result_t ion_ici_get_large_pool_alloc(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.largePoolAllocated), sizeof(state.largePoolAllocated), &(result.length));

	return result;
}

expr_result_t ion_ici_get_unused_size(Lyst params)
{
	SdrnmState state;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	sdrnm_state_get(&state);
	result.value = adm_copy_integer((uint8_t*)&(state.unusedSize), sizeof(state.unusedSize), &(result.length));

	return result;
}



/* ION INDUCT */

/*
expr_result_t ion_induct_get_all(Lyst params)
{
	datacol_entry_t *entry = (datacol_entry_t*)lyst_data(lyst_first(params));
	unsigned long val = 0;
	char name[256];
	expr_result_t result;
	result.type = EXPR_TYPE_BLOB;

	NmbpInduct induct;
	int success = 0;

	memset(name,'\0',256);
	memcpy(name,entry->value, entry->length);

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.length = sizeof(NmbpInduct);
		result.value = (uint8_t*) STAKE(result.length);
		memset(result.value, 0, result.length);
		memcpy(result.value, &induct, result.length);
	}

	return result;
}

expr_result_t ion_induct_get_name(Lyst params)
{
	datacol_entry_t *entry = (datacol_entry_t*)lyst_data(lyst_first(params));
	unsigned long val = 0;
	char name[256];
	expr_result_t result;
	result.type = EXPR_TYPE_STRING;

	NmbpInduct induct;
	int success = 0;

	memset(name,'\0',256);
	memcpy(name,entry->value, entry->length);

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.length = (uint64_t) strlen(induct.inductName) + 1;
		result.value = (uint8_t*) STAKE(result.length);
		memset(result.value, 0, result.length);
		memcpy(result.value, induct.inductName, result.length);
	}

	return result;
}

expr_result_t ion_induct_get_last_reset(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.lastResetTime),
			                 sizeof(induct.lastResetTime),
			                 &(result.length));
	}
	return result;
}

expr_result_t ion_induct_get_rx_bndl(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleRecvCount),
			                 sizeof(induct.bundleRecvCount),
			                 &(result.length));
	}
	return result;
}


expr_result_t ion_induct_get_rx_byte(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleRecvBytes),
			                 sizeof(induct.bundleRecvBytes),
			                 &(result.length));
	}
	return result;
}

expr_result_t ion_induct_get_mal_bndl(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleMalformedCount),
			                 sizeof(induct.bundleMalformedCount),
			                 &(result.length));
	}
	return result;
}


expr_result_t ion_induct_get_mal_byte(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleMalformedBytes),
			                 sizeof(induct.bundleMalformedBytes),
			                 &(result.length));
	}
	return result;
}

expr_result_t ion_induct_get_inauth_bndl(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleInauthenticCount),
			                 sizeof(induct.bundleInauthenticCount),
			                 &(result.length));
	}
	return result;
}


expr_result_t ion_induct_get_inauth_byte(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleInauthenticBytes),
			                 sizeof(induct.bundleInauthenticBytes),
			                 &(result.length));
	}
	return result;
}

expr_result_t ion_induct_get_over_bndl(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleOverflowCount),
			                 sizeof(induct.bundleOverflowCount),
			                 &(result.length));
	}
	return result;
}


expr_result_t ion_induct_get_over_byte(Lyst params)
{
	char *name = (char *) lyst_data(lyst_first(params));
	NmbpInduct induct;
	int success = 0;
	expr_result_t result;
	result.type = EXPR_TYPE_UINT32;

	result.length = 0;
	result.value = NULL;
	bpnm_induct_get(name, &induct, &success);
	if(success != 0)
	{
		result.value = adm_copy_integer((uint8_t*)&(induct.bundleOverflowBytes),
			                 sizeof(induct.bundleOverflowBytes),
			                 &(result.length));
	}
	return result;
}
*/

/* ION NODE */
/*
expr_result_t ion_node_get_all(Lyst params)
{
	expr_result_t inducts;
	expr_result_t outducts;
	expr_result_t result;
	result.type = EXPR_TYPE_BLOB;

	inducts = ion_node_get_inducts(params);
	outducts = ion_node_get_outducts(params);

	result.length = inducts.length + outducts.length;
	result.value = (uint8_t*) STAKE(result.length);
	memcpy(result.value,inducts.value,inducts.length);
	memcpy(result.value + inducts.length, outducts.value, outducts.length);
	expr_release(inducts);
	expr_release(outducts);

	return result;
}
*/
expr_result_t ion_node_get_inducts(Lyst params)
{

	char names[2048];
	char *ptrs[128];
	int num = 0;
	Sdnv nm_sdnv;
	uint8_t *cursor = NULL;
	expr_result_t result;
	result.type = EXPR_TYPE_BLOB;

//	bpnm_inductNames_get((char *) names, ptrs, &num);

	encodeSdnv(&nm_sdnv, num);

	result.length = nm_sdnv.length +        /* NUM as SDNV length */
			        strlen(ptrs[num-1]) +   /* length of last string */
			        (ptrs[num-1] - names) + /* # bytes to get to last string */
			        1;                      /* Final NULL terminator. */
	result.value = (uint8_t *) STAKE(result.length);

	cursor = result.value;

	memcpy(cursor,nm_sdnv.text, nm_sdnv.length);
	cursor += nm_sdnv.length;

	memcpy(cursor, names, result.length - nm_sdnv.length);

	return result;
}


expr_result_t ion_node_get_plans(Lyst params)
{

	Sdr             sdr = getIonsdr();

	Object elt;
	Object ductObj;
	OBJ_POINTER(IpnPlan,curPlan);
	OBJ_POINTER(Outduct,curDuct);
	OBJ_POINTER(ClProtocol,curCLA);
	uint32_t resultLen;
	char* destDuctName=(char*)STAKE(SDRSTRING_BUFSZ);
	int ductLen;
	Lyst outdatacol = lyst_create();
	expr_result_t result;

	if (bpAttach() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_plan_add","can't attach to BP", NULL);
		SRELEASE(destDuctName);
		return result;
	}

	if (ipnInit() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_plan_add","can't initialize routing database", NULL);
		SRELEASE(destDuctName);
		return result;
	}

	result.type=EXPR_TYPE_BLOB;

	sdr_begin_xn(sdr);
	for (elt = sdr_list_first(sdr,(getIpnConstants())->plans); elt; elt = sdr_list_next(sdr, elt))
	{
		GET_OBJ_POINTER(sdr,IpnPlan,curPlan,sdr_list_data(sdr,elt));
		ductObj = sdr_list_data(sdr,curPlan->defaultDirective.outductElt);
		GET_OBJ_POINTER(sdr,Outduct,curDuct,ductObj);
		GET_OBJ_POINTER(sdr,ClProtocol,curCLA,curDuct->protocol);

        datalist_t planDL=datalist_create(NULL);

		datalist_insert_with_type(&planDL,DLIST_TYPE_UVAST,&curPlan->nodeNbr);
		datalist_insert_with_type(&planDL,DLIST_TYPE_STRING,&curCLA->name,strlen(curCLA->name));
		datalist_insert_with_type(&planDL,DLIST_TYPE_STRING,&curDuct->name,strlen(curDuct->name));

		//Read ductname from string
		ductLen=sdr_string_read(sdr,destDuctName,curPlan->defaultDirective.destDuctName);
		if(ductLen==-1)
			continue;

		datalist_insert_with_type(&planDL,DLIST_TYPE_STRING,destDuctName,ductLen);

		datacol_entry_t* dlSerialized = datalist_serialize_to_datacol(&planDL);

		lyst_insert_last(outdatacol,dlSerialized);
	}
	sdr_exit_xn(sdr);

	result.value=utils_datacol_serialize(outdatacol,&resultLen);
	result.length=(uint32_t)resultLen;

	return result;

}

expr_result_t ion_node_get_groups(Lyst params)
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


/*	Control functions	*/

uint32_t ion_ctrl_induct_reset(Lyst params)
{
 /* TODO: Implement. */
	return 0;
}

uint32_t ion_ctrl_outduct_reset(Lyst params)
{
	/* TODO: Implement */
	return 0;
}

uint32_t ion_ctrl_plan_add(Lyst params)
{

	LystElt elt = 0;
	unsigned short lystIdx = 0;
	char* outductName = "*";
	char protocolName[D_ION_ADM_PROTONAME_SIZE];
	char ipName[D_ION_ADM_IPNAME_SIZE];
	uint32_t port;
	DuctExpression ductExp;
	uvast nodeName;
	VOutduct	*vduct;
	PsmAddress vductElt;
	uint32_t datacolSize;
	DTNMP_DEBUG_ENTRY("ion_ctrl_plan_add","()",NULL);

	if (bpAttach() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_plan_add","Can't attach to BP SM", NULL);
		return -1;
	}

	if (ipnInit() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_plan_add","Cannot init routing database", NULL);
		return -1;
	}

	//Parameter handling
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);

	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("ion_ctrl_plan_add","Deserialized %d bytes to datalist",datacolSize);

		CHKVALID(datalist_get(&curDl,0,&nodeName,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&protocolName[0],NULL,DLIST_TYPE_STRING));
		CHKVALID(datalist_get(&curDl,2,&ipName[0],NULL,DLIST_TYPE_STRING));
		CHKVALID(datalist_get(&curDl,3,&port,NULL,DLIST_TYPE_UINT32));

		findOutduct(protocolName, outductName, &vduct, &vductElt);

		if(vductElt==0)
		{
			DTNMP_DEBUG_ERR("ion_ctrl_plan_add","Could not find outduct",NULL);
			datalist_free_contents(&curDl);
			continue;
		}

		DTNMP_DEBUG_WARN("ion_ctrl_plan_add","Adding %d \"%s\" \"%s\" %d %s",nodeName,protocolName,ipName,port,vduct->protocolName);

		ductExp.outductElt=vduct->outductElt;
		ductExp.destDuctName=(char*)malloc(128);
		sprintf(ductExp.destDuctName,"%s:%d",ipName,port);
		DTNMP_DEBUG_INFO("ion_ctrl_plan_add","Adding ductname \"%s\"",ductExp.destDuctName);
		//Add the plan
		if(ipn_addPlan(nodeName,&ductExp)<=0)
		{
			DTNMP_DEBUG_ERR("ion_ctrl_plan_add","Adding plan failed",NULL);
			datalist_free_contents(&curDl);
			continue;
		}
		//datalist_free_contents(&curDl);
		datalist_free_contents(&curDl);
	}
	utils_datacol_destroy(&dlDatacol);
	return 0;
}

uint32_t ion_ctrl_plan_remove(Lyst params)
{
	uvast nodeName;
	uint32_t datacolSize;
	if (bpAttach() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_plan_add","can't attach to BP", NULL);
		return -1;
	}

	if (ipnInit() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_plan_add","can't initialize routing database", NULL);
		return -1;
	}
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);

	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("ion_ctrl_plan_remove","Deserialized %d bytes to datalist",datacolSize);

		CHKVALID(datalist_get(&curDl,0,&nodeName,NULL,DLIST_TYPE_UVAST));
	//char* value = (char*)((datacol_entry_t *)lyst_data(lyst_first(params)))->value;
		DTNMP_DEBUG_INFO("ion_ctrl_plan_remove","Deleting %d",nodeName);
		//Do the work
		if(ipn_removePlan(nodeName)<=0)
			DTNMP_DEBUG_ERR("ion_ctrl_plan_remove","Couldn't remove entry",NULL);

		datalist_free_contents(&curDl);
	}
	utils_datacol_destroy(&dlDatacol);
	return 0;
}

uint32_t ion_ctrl_group_add(Lyst params)
{
	uvast startNode=0;
	uvast endNode=0;
	char viaNode[MAX_EID_LEN];
	uint32_t datacolSize;
	VOutduct	*vduct;
	PsmAddress vductElt;

	DTNMP_DEBUG_ENTRY("ion_ctrl_group_add","()",NULL);
	//Sanity check
	if(lyst_length(params)<1) //Not enough params
	{
		return 0; //Check for proper return codes
	}
	//Parameter handling
	if (bpAttach() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_group_add","ipnadmin can't attach to BP", NULL);
		return -1;
	}

	if (ipnInit() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_group_add","ipnadmin can't initialize routing database", NULL);
		return -1;
	}
	//Lyst datacol=lyst_create();
	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);
	DTNMP_DEBUG_INFO("ion_ctrl_group_add","Deserialized %d bytes to datalist param: %d",bytesUsed,lyst_length(params));
	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("ion_ctrl_group_add","Deserialized %d bytes to datalist",datacolSize);

		CHKVALID(datalist_get(&curDl,0,&startNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&endNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,2,&viaNode,NULL,DLIST_TYPE_STRING));

		//Do the work
		DTNMP_DEBUG_INFO("ion_ctrl_group_add","%d %d %s",startNode,endNode,viaNode);
		ipn_addGroup(startNode,endNode,(char*)viaNode);

		datalist_free_contents(&curDl);
	}
	utils_datacol_destroy(&dlDatacol);

	return 0;

}

uint32_t ion_ctrl_group_remove(Lyst params)
{
	LystElt elt = 0;
	unsigned short lystIdx = 0;
	uvast startNode;
	uvast endNode;
	VOutduct	*vduct;
	PsmAddress vductElt;
	uint32_t datacolSize;
	DTNMP_DEBUG_INFO("ion_ctrl_group_remove","()",NULL);
	//Sanity check
	if(lyst_length(params)<1) //Not enough params
	{
		return 0; //Check for proper return codes
	}
	//Parameter handling
	if (bpAttach() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_group_add","ipnadmin can't attach to BP", NULL);
		return -1;
	}

	if (ipnInit() < 0)
	{
		DTNMP_DEBUG_ERR("ion_ctrl_group_add","ipnadmin can't initialize routing database", NULL);
		return -1;
	}

	uint32_t bytesUsed;
	datacol_entry_t* paramCol = (datacol_entry_t*)lyst_data(lyst_first(params));
	Lyst dlDatacol=utils_datacol_deserialize((uint8_t*)paramCol->value,paramCol->length,&bytesUsed);

	for(LystElt datalistElt = lyst_first(dlDatacol) ; datalistElt ; datalistElt=lyst_next(datalistElt))
	{
		datacol_entry_t* datacol = (datacol_entry_t*)lyst_data(datalistElt);
		datalist_t curDl = datalist_deserialize_from_buffer(datacol->value,datacol->length,&datacolSize);
		DTNMP_DEBUG_INFO("ion_ctrl_group_remove","Deserialized %d bytes to datalist",datacolSize);

		CHKVALID(datalist_get(&curDl,0,&startNode,NULL,DLIST_TYPE_UVAST));
		CHKVALID(datalist_get(&curDl,1,&endNode,NULL,DLIST_TYPE_UVAST));

		//Do the work
		DTNMP_DEBUG_INFO("ion_ctrl_group_remove","%d %d",startNode,endNode);
		ipn_removeGroup(startNode,endNode);
		datalist_free_contents(&curDl);
	}
	DTNMP_DEBUG_INFO("ion_ctrl_group_remove","%d %d",startNode,endNode);

	utils_datacol_destroy(&dlDatacol);
	return 0;
}
//#endif /* _HAVE_ION_ADM_ */
