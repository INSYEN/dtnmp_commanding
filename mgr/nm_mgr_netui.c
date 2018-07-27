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
 ** \file nm_mgr_netui.c
 **
 **
 ** Description: A rpc-based DTNMP Manager.
 **
 ** Notes:
 **		1. Currently we do not support ACLs.
 **		2. When defining new MID, add to list so it can be IDX selected.
 **
 ** Assumptions:
 **
 ** Modification History:
 **  MM/DD/YY  AUTHOR         DESCRIPTION
 **  --------  ------------   ---------------------------------------------
 **  01/18/13  E. Birrane     Code comments and cleanup
 **  06/25/13  E. Birrane     Removed references to priority field. Add ISS flag.
 **  06/25/13  E. Birrane     Renamed message "bundle" message "group".
 **	 03/01/15  J.P. Mayer	  Copied and renamed
 **  03/15/15  J.P. Mayer	  Implemented RPC system and datalists
 *****************************************************************************/

#include "ctype.h"

#include "platform.h"

#include "nm_mgr_netui.h"
#include "nm_mgr_names.h"
#include "nm_mgr_print.h"
#include "mgr_db.h"

#include "../shared/utils/utils.h"
#include "../shared/adm/adm.h"
#include "../shared/adm/adm_agent.h"
#include "../shared/primitives/ctrl.h"
#include "../shared/primitives/rules.h"
#include "../shared/primitives/mid.h"
#include "../shared/primitives/oid.h"
#include "../shared/msg/pdu.h"
#include "../shared/msg/msg_ctrl.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

Lyst gParmSpec;


static void mgr_signal_handler()
{
 	gRunning = 0;
}



/******************************************************************************
 *
 * \par Function Name: ui_clear_reports
 *
 * \par Clears the list of received data reports from an agent.
 *
 * \par Notes:
 *	\todo - Add ability to clear reports from a particular agent, or of a
 *	\todo   particular type, or for a particular timeframe.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  08/10/11  V.Ramachandran Initial implementation,
 *  01/18/13  E. Birrane     Debug updates.
 *  04/18/13  V.Ramachandran Multiple-agent support (added param)
 *****************************************************************************/
void ui_clear_reports(agent_t* agent)
{
    if(agent == NULL)
    {
    	AMP_DEBUG_ENTRY("ui_clear_reports","(NULL)", NULL);
    	AMP_DEBUG_ERR("ui_clear_reports", "No agent specified.", NULL);
        AMP_DEBUG_EXIT("ui_clear_reports","->.",NULL);
        return;
    }
    AMP_DEBUG_ENTRY("ui_clear_reports","(%s)",agent->agent_eid.name);

	int num = lyst_length(agent->reports);
	rpt_clear_lyst(&(agent->reports), NULL, 0);
	g_reports_total -= num;

	AMP_DEBUG_ALWAYS("ui_clear_reports","Cleared %d reports.", num);
    AMP_DEBUG_EXIT("ui_clear_reports","->.",NULL);
}



/******************************************************************************XXX
 *
 * \par Function Name: netui_construct_ctrl_by_idx
 *
 * \par Constructs a "execute control" message by selecting the control from
 *      a list of controls. Puts the control in a PDU and sends to agent.
 *
 * \par Notes:
 *	\todo Add ability to select which agent, and to apply ACL.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *  04/18/13  V.Ramachandran Multiple-agent support (added param)
 *  06/25/13  E. Birrane     Renamed message "bundle" message "group".
 *  03/01/15  J.P. Mayer	 Migrated to cmdFormat
 *****************************************************************************/

void netui_construct_ctrl_by_idx(agent_t* agent,cmdFormat* curCmd)
{
	char line[256];
	uint32_t offset;
	char mid_str[256];
	Lyst mids;
	uint32_t size = 0;

	if(agent == NULL)
	{
		AMP_DEBUG_ERR("netui_construct_ctrl_by_idx", "No agent specified.", NULL);
		return;
	}
	AMP_DEBUG_INFO("netui_construct_ctrl_by_idx","(%s %d)", curCmd->eid,curCmd->numChunks);



	/* Step 1: Parse the user input. */
	int midIdx = netui_find_ctrl_idx_by_name((curCmd->cmdChunks[curCmd->numChunks]));

	if(midIdx==-1)
	{
		AMP_DEBUG_ERR("netui_construct_ctrl_by_idx","Couldn't find ADM %d",curCmd->numChunks);
		return;
	}
	sprintf(mid_str,"%d",midIdx);

	mids = netui_parse_mid_str(curCmd,mid_str, lyst_length(gAdmCtrls)-1, MID_CONTROL);

	if (lyst_length(mids) > 1)
	{
        AMP_DEBUG_ERR("netui_construct_ctrl_by_idx",
            "Got %d MIDs, but CTRL message just supports one MID.",
            lyst_length(mids));
    }

    /* Step 2: Construct the control primitive. */
    msg_perf_ctrl_t *ctrl = msg_create_perf_ctrl(0, mids);

    /* Step 3: Construct a PDU to hold the primitive. */
    uint8_t *data = msg_serialize_perf_ctrl(ctrl, &size);

	pdu_msg_t *pdu_msg = pdu_create_msg(MSG_TYPE_CTRL_EXEC, data, size, NULL);
	pdu_group_t *pdu_group = pdu_create_group(pdu_msg);

	/* Step 4: Send the PDU. */
	iif_send(&ion_ptr, pdu_group, agent->agent_eid.name);

	/* Step 5: Release remaining resources. */
	pdu_release_group(pdu_group);
	msg_destroy_perf_ctrl(ctrl);
	midcol_destroy(&mids);

	AMP_DEBUG_EXIT("netui_construct_ctrl_by_idx","->.", NULL);
}



/******************************************************************************XXX
 *
 * \par Function Name: netui_construct_time_rule_by_idx
 *
 * \par Constructs a "time production report" control by selecting data from
 *      a list of MIDs. Puts the control in a PDU and sends to agent.
 *
 * \par Notes:
 *	\todo Add ability to select which agent, and to apply ACL.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *  06/25/13  E. Birrane     Renamed message "bundle" message "group".
 *  03/01/15  J.P. Mayer	 Migrated to cmdFormat
 *****************************************************************************/

void netui_construct_time_rule_by_idx(agent_t* agent,cmdFormat* curCmd)
{
	time_t offset = 0;
	uint32_t period = 0;
	uint32_t evals = 0;
	char mid_str[256];
	Lyst mids = lyst_create();
	uint32_t size = 0;
	uint8_t numArgs=0;
	char* command;
	char* cursor;
	char* arguments;
	if(agent == NULL)
	{
		AMP_DEBUG_ENTRY("ui_construct_time_rule_by_idx","(NULL)", NULL);
		AMP_DEBUG_ERR("ui_construct_time_rule_by_idx", "Null EID", NULL);
		AMP_DEBUG_EXIT("ui_construct_time_rule_by_idx","->.", NULL);
		return;
	}
	AMP_DEBUG_INFO("ui_construct_time_rule_by_idx","(%s)", agent->agent_eid.name);

	/* Step 1: Read and parse the rule. */
	/* Step 1a: "tokenize" the string */

	char** args=netui_parse_arguments(curCmd->arguments,&numArgs);
	if(numArgs<4)
	{
		AMP_DEBUG_ERR("netui_construct_time_rule_by_idx","Not enough arguments %d",numArgs);
		return;
	}

	offset=atoi(args[0]);
	period=atoi(args[1]);
	evals=atoi(args[2]);

	/* Step 1b: Split the MID array */
	unsigned int x = 3;

	for(x=3;x<numArgs;x++)
	{
		AMP_DEBUG_INFO("netui_construct_time_rule_by_idx","Found argument %s",args[x]);
		//Fill into temp command
		char* arrayStart = strchr(args[x],'{');
		if(arrayStart==NULL)
		{
			AMP_DEBUG_ERR("netui_construct_time_rule_by_idx","Couldn't find array start",NULL);
			break; //Keep going with what we have
		}
		char* arrayEnd = strchr(arrayStart,'}');
		if(arrayEnd==NULL)
		{
			AMP_DEBUG_ERR("netui_construct_time_rule_by_idx","Couldn't find array end %s",arrayStart);
			break; //Keep going with what we have
		}
		//Delimit array
		arrayEnd[0]='\0';
		cursor=strchr(arrayStart,',');

		if(cursor==NULL)
			command=arrayStart+1;
		else
		{
			cursor[0]='\0';
			command = arrayStart+1;
			arguments = &cursor[0]+1;
		}
		//This line is a expensive hack and should be changed
		int mididx = netui_find_data_idx_by_name(command);
		if(mididx==-1)
			break;
		snprintf(&mid_str[0],256,"%d",mididx);
		//Insert into lyst

		netui_parse_single_mid_str(mids, mid_str,arguments,lyst_length(gAdmData)-1, MID_ATOMIC);


		//mids = netui_parse_mid_str(curCmd,midIdxC, lyst_length(gAdmData))-1, MID_TYPE_DATA)
	}

/*	mids = netui_parse_mid_str(curCmd,midIdxC, lyst_length(gAdmData))-1, MID_TYPE_DATA);

	/* Step 2: Construct the control primitive. */
	trl_t *entry = trl_create(NULL, offset, evals, period, mids); // XXX: Place some MID ID here

	/* Step 3: Construct a PDU to hold the primitive. */
	uint8_t *data = trl_serialize(entry, &size);
	pdu_msg_t *pdu_msg = pdu_create_msg(MSG_TYPE_CTRL_PERIOD_PROD, data, size, NULL);
	pdu_group_t *pdu_group = pdu_create_group(pdu_msg);

	/* Step 4: Send the PDU. */
	iif_send(&ion_ptr, pdu_group, agent->agent_eid.name);

	/* Step 5: Release remaining resources. */
	pdu_release_group(pdu_group);
	trl_release(entry);
	midcol_destroy(&mids);

	SRELEASE(args);

	AMP_DEBUG_EXIT("ui_construct_time_rule_by_idx","->.", NULL);
}

/******************************************************************************
 *
 * \par Function Name: netui_register_agent
 *
 * \par Register a new agent.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/15/15  J.P. Mayer	 Initial Implementation
 *****************************************************************************/
void netui_register_agent(cmdFormat* curCmd)
{
	eid_t agent_eid;

	AMP_DEBUG_ENTRY("register_agent", "()", NULL);

	sscanf(curCmd->eid, "%s", agent_eid.name);
	mgr_agent_add(agent_eid);

	AMP_DEBUG_EXIT("register_agent", "->.", NULL);
}

/******************************************************************************
 *
 * \par Function Name: ui_deregister_agent
 *
 * \par Remove and deallocate an agent.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  04/23/13  V.Ramachandran Initial Implementation
 *****************************************************************************/
void ui_deregister_agent(agent_t* agent)
{
	AMP_DEBUG_ENTRY("ui_deregister_agent","(%llu)", (unsigned long)agent);

	if(agent == NULL)
	{
		AMP_DEBUG_ERR("ui_deregister_agent", "No agent specified.", NULL);
		AMP_DEBUG_EXIT("ui_deregister_agent","->.",NULL);
		return;
	}
	AMP_DEBUG_ENTRY("ui_deregister_agent","(%s)",agent->agent_eid.name);

	lockResource(&agents_mutex);

	if(mgr_agent_remove(&(agent->agent_eid)) < 1)
	{
		AMP_DEBUG_WARN("ui_deregister_agent","No agent by that name is currently registered.\n", NULL);
	}
	else
	{
		AMP_DEBUG_ALWAYS("ui_deregister_agent","Successfully deregistered agent.\n", NULL);
	}

	unlockResource(&agents_mutex);
}

/******************************************************************************
 *
 * \par Function Name: ui_eventLoop
 *
 * \par Main event loop for the UI thread.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/
void ui_eventLoop(int *running)
{
	AMP_DEBUG_ERR("uiThread","running",NULL);

//	configure sockets
	int conSock = socket(AF_INET,SOCK_STREAM,0);
	int dataSock = 0;
	short moreData=0;
	socklen_t peerAddrSize = sizeof(struct sockaddr_in);
	struct sockaddr_in localAddr,peerAddr;
	unsigned short port = 12345;
	char cmdBuffer[D_CMDBUFSIZE];
	char* commandReentry=&cmdBuffer[0];

    struct sigaction a;

    sigset_t intmask;
    sigemptyset(&intmask);
    sigaddset(&intmask, SIGINT);
    sigprocmask(SIG_UNBLOCK, &intmask, NULL);

    a.sa_handler = mgr_signal_handler;
    a.sa_flags = 0;
    sigemptyset( &a.sa_mask );
    sigaction( SIGINT, &a, NULL );

	if(conSock == -1)
	{
		printf("Socket error: Could not create initial listener\n");
		return;
	}
	reUseAddress(conSock);
	memset(&localAddr,0,sizeof(struct sockaddr_in));

	localAddr.sin_family=AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(port);

	if(bind(conSock,(struct sockaddr*)&localAddr,sizeof(struct sockaddr_in))==-1)
	{
		printf("Socket error: Could not bind\n");
		return;
	}

	if(listen(conSock,1024)==-1)
	{
		printf("Socket error: Could not listen\n");
		return;
	}
	//Fix sigpipe behavior
	signal(SIGPIPE,SIG_IGN); //Danke.
	reUseAddress(conSock);

	while(*running)
	{
		//Check for connections, or use valid one
		if(dataSock==0)
		{
			AMP_DEBUG_INFO("ui_eventloop","Waiting for connection...",NULL);
			dataSock=accept(conSock,(struct sockaddr*)&peerAddr,&peerAddrSize);
			if(dataSock==-1)
			{
				printf("could not create data socket\n");
				return;
			}
			else
			{
				AMP_DEBUG_INFO("ui_eventloop","connected",NULL);
				struct timeval tv={1,0};
				if(setsockopt(dataSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval)) == -1)
				{
					AMP_DEBUG_ERR("netui_eventloop","Could not set receive timeout",NULL);
					return;
				}
				reUseAddress(dataSock);
			}
		}


		//We should now have a valid connection.
		//Send any variables waiting in the queue
		lockResource(&variable_queue_mutex);
		LystElt elt,lastElt;

		if(lyst_length(variable_queue))
		{
			AMP_DEBUG_ERR("netui_eventloop","Found variables",NULL);
			for(elt = lyst_first(variable_queue);elt;elt=lyst_next(elt))
			{
				//This should be moved to a function, eventually
				char* varFmtStr;
				char* varTextRep;
				char** varToPrint;
				char* varValue=(char*)STAKE(D_VARENTRYSIZE);
				char* varBuffer=(char*)STAKE(D_VARSTRSIZE);
				variableQueueEntry* curEntry = (variableQueueEntry*)lyst_data(elt);

				if(curEntry==NULL)
				{
					AMP_DEBUG_ERR("netui_eventloop","Invalid data pointer",NULL);
					continue;
				}


				switch(curEntry->type)
				{
					case TYPE_INT32: netui_print_int32(curEntry->value,curEntry->size,varValue);varTextRep="int32";break;
					case TYPE_UINT32: netui_print_uint32(curEntry->value,curEntry->size,varValue);varTextRep="uint32"; break;
					case TYPE_STRING: netui_print_string(curEntry->value,curEntry->size,varValue); varTextRep="string";break;
					case TYPE_TDC: netui_print_tdc(curEntry->value,curEntry->size,varValue); varTextRep="tdc";break;
					default: AMP_DEBUG_ERR("netui_eventloop", "Type not supported: %u\n", curEntry->type); continue;
				}


				AMP_DEBUG_INFO("netui_eventloop","Variable: %s",curEntry->name);
				uint16_t varStrSize = snprintf(varBuffer,D_VARSTRSIZE,"v:%s@%lu\\%s(%s)=%s;\n",curEntry->producer_eid.name,curEntry->timestamp,curEntry->name,varTextRep,varValue);



				//Free memory
				SRELEASE(curEntry->value);
				SRELEASE(curEntry->name);
				SRELEASE(curEntry);

				lastElt = lyst_prev(elt);
				lyst_delete(elt);
				elt=lastElt;

				//Transmit
				send(dataSock,varBuffer,varStrSize,0);

				SRELEASE(varValue);
				SRELEASE(varBuffer);
			}
		}
		unlockResource(&variable_queue_mutex);

		//Check if we have (properly formatted) data
		int sockstatus=0;
		if(!moreData)
		{
			memset(&cmdBuffer,'\0',D_CMDBUFSIZE);
			//AMP_DEBUG_INFO("netui_eventloop","Grabbing additional data",NULL);
			sockstatus = recv(dataSock,&cmdBuffer,D_CMDBUFSIZE-2,0);
			if(sockstatus > 0)
			{
				cmdBuffer[sockstatus+1]='\0';
				sockstatus=send(dataSock,&cmdBuffer,sockstatus+1,0);
			}

			if(sockstatus<=0)
			{
				if((errno==EWOULDBLOCK|EAGAIN)&(sockstatus!=0)) //We just don't have data
				{
					//AMP_DEBUG_INFO("netui_eventloop","No data, continuing",NULL);
					continue;
				}
				close(dataSock);
				dataSock=0;
				AMP_DEBUG_INFO("netui_eventloop","Socket broken, continuing",NULL);
				continue;
			}

			commandReentry=&cmdBuffer[0];
		}

		cmdFormat* curCmd=netui_create_cmdformat();
		moreData = netui_build_command(&commandReentry,curCmd,sizeof(cmdFormat));

		if(moreData == 0)
		{
			netui_free_cmdformat(curCmd);
			AMP_DEBUG_WARN("netui_eventLoop","Invalid command",NULL);
			continue;
		}

		short cmdIdx = 0;
		char* curChunk = (curCmd->cmdChunks[cmdIdx]);

		if(&curChunk==NULL)
		{
			netui_free_cmdformat(curCmd);
			continue;
		}
		AMP_DEBUG_INFO("netui_eventLoop","command has %d tokens, first is \"%s\" %p",curCmd->numChunks+1,(char*)(curCmd->cmdChunks[cmdIdx]),&(curCmd->cmdChunks[cmdIdx]));

		/* notes on top level structure:
		<agent>.manager. - For reg/dereg
		<agent>.reports. - For reporting
		<agent>.controls. - For control transmission
		*/
		NETUI_SECTION("manager")
		{
			NETUI_DEF_ACTION("register",netui_register_agent(curCmd));
			NETUI_DEF_ACTION("list",ui_print_agents());
			NETUI_DEF_ACTION("deregister",ui_deregister_agent(netui_find_agent_by_name(curCmd->eid)));
		}

		NETUI_SECTION("controls")
		{
			NETUI_DEF_ACTION("create",netui_construct_ctrl_by_idx(netui_find_agent_by_name(curCmd->eid),curCmd));
		}

		NETUI_SECTION("debug")
		{
			NETUI_DEF_ACTION("controls",ui_list_ctrls());
			NETUI_DEF_ACTION("datarefs",ui_list_atomic());
		}

		NETUI_SECTION("reports")
		{
			AMP_DEBUG_INFO("netui_eventloop","In reports",NULL);
			NETUI_DEF_ACTION("time",netui_construct_time_rule_by_idx(netui_find_agent_by_name(curCmd->eid),curCmd));
			NETUI_DEF_ACTION("show",netui_print_reports(netui_find_agent_by_name(curCmd->eid)));
			NETUI_DEF_ACTION("number",netui_get_num_reports_by_agent());
			NETUI_DEF_ACTION("delete",ui_clear_reports(netui_find_agent_by_name(curCmd->eid)));
		}

		netui_free_cmdformat(curCmd);

	}

    AMP_DEBUG_ALWAYS("netui_eventLoop", "Exiting.", NULL);
    AMP_DEBUG_EXIT("netui_eventLoop","->.", NULL);
    pthread_exit(NULL);
}

/*
 * No double-checking, assumes code is correct...
 */
void ui_add_parmspec(char *mid_str,
						       uint8_t num,
		                       char *n1, uint8_t p1,
		                       char *n2, uint8_t p2,
		                       char *n3, uint8_t p3,
		                       char *n4, uint8_t p4,
		                       char *n5, uint8_t p5)
{
	ui_parm_spec_t *spec = STAKE(sizeof(ui_parm_spec_t));
	CHKVOID(spec);

	memset(spec, 0, sizeof(ui_parm_spec_t));

	spec->mid = mid_from_string(mid_str);
	spec->num_parms = num;

	if(n1 != NULL) istrcpy(spec->parm_name[0], n1, MAX_PARM_NAME);
	spec->parm_type[0] = p1;

	if(n2 != NULL) istrcpy(spec->parm_name[1], n2, MAX_PARM_NAME);
	spec->parm_type[1] = p2;

	if(n3 != NULL) istrcpy(spec->parm_name[2], n3, MAX_PARM_NAME);
	spec->parm_type[2] = p3;

	if(n4 != NULL) istrcpy(spec->parm_name[3], n4, MAX_PARM_NAME);
	spec->parm_type[3] = p4;

	if(n5 != NULL) istrcpy(spec->parm_name[4], n5, MAX_PARM_NAME);
	spec->parm_type[4] = p5;

	lyst_insert_last(gParmSpec, spec);
}

ui_parm_spec_t* ui_get_parmspec(mid_t *mid)
{
	ui_parm_spec_t *result = NULL;

	LystElt elt;

	for(elt = lyst_first(gParmSpec); elt; elt = lyst_next(elt))
	{
		result = lyst_data(elt);

		if(mid_compare(mid, result->mid, 0) == 0)
		{
			return result;
		}
	}

	return NULL;
}


/*
 * We need to find out a description for the entry so we can print it out.
 * So, if entry is <RPT MID> <int d1><int d2><int d3> we need to match the items
 * to elements of the report definition.
 *
 */
void netui_print_entry(rpt_entry_t *entry, eid_t *receiver, time_t ts, uvast *mid_sizes, uvast *data_sizes)
{
	LystElt elt = NULL;
	def_gen_t *cur_def = NULL;
	uint8_t del_def = 0;

	if((entry == NULL) || (mid_sizes == NULL) || (data_sizes == NULL))
	{
		AMP_DEBUG_ERR("netui_print_entry","Bad Args.", NULL);
		return;
	}

	/* Step 1: Calculate sizes...*/
    *mid_sizes = *mid_sizes + entry->id->raw_size;

    for(elt = lyst_first(entry->contents->datacol); elt; elt = lyst_next(elt))
    {
    	blob_t *cur = lyst_data(elt);
        *data_sizes = *data_sizes + cur->length;
    }
    *data_sizes = *data_sizes + entry->contents->hdr.length;

	/* Step 1: Print the MID associated with the Entry. */
    printf(" (");
    ui_print_mid(entry->id);
	printf(") has %d values. ", entry->contents->hdr.length);


    /*
     * Step 2: Try and find the metadata associated with each
     *         value in the TDC. Since the TDC is already typed, the
     *         needed meta-data information is simply the
     *         "name" of the data.
     *
     *         i Only computed data definitions, reports, and macros
     *         need names. Literals, controls, and atomic data do
     *         not (currently) define extra meta-data for their
     *         definitions.
     *
     *         \todo: Consider printing names for each return
     *         value from a control.
     */

    cur_def = NULL;

	if(MID_GET_FLAG_ID(entry->id->flags) == MID_ATOMIC)
	{
		adm_datadef_t *ad_entry = adm_find_datadef(entry->id);

		/* Fake a def_gen_t for now. */
		if(ad_entry != NULL)
		{
	    	Lyst tmp = lyst_create();
	    	lyst_insert(tmp,mid_copy(ad_entry->mid));
	    	cur_def = def_create_gen(mid_copy(ad_entry->mid), ad_entry->type, tmp);
	    	del_def = 1;
		}
	}
	else if(MID_GET_FLAG_ID(entry->id->flags) == MID_COMPUTED)
	{
		var_t *cd = NULL;
	    if(MID_GET_FLAG_ISS(entry->id->flags) == 0)
	    {
	    	cd = var_find_by_id(gAdmComputed, NULL, entry->id);
	    }
	    else
	    {
	    	cd = var_find_by_id(gMgrVDB.compdata, &(gMgrVDB.compdata_mutex), entry->id);
	    }

	    // Fake a def_gen just for this CD item.
	    if(cd != NULL)
	    {
	    	Lyst tmp = lyst_create();
	    	lyst_insert(tmp,mid_copy(cd->id));
	    	cur_def = def_create_gen(mid_copy(cd->id), cd->value.type, tmp);
	    	del_def = 1;
	    }
	}
	else if(MID_GET_FLAG_ID(entry->id->flags) == MID_REPORT)
	{
	    if(MID_GET_FLAG_ISS(entry->id->flags) == 0)
	    {
	    	cur_def = def_find_by_id(gAdmRpts, NULL, entry->id);
	    }
	    else
	    {
	    	cur_def = def_find_by_id(gMgrVDB.reports, &(gMgrVDB.reports_mutex), entry->id);
	    }
	}
	else if(MID_GET_FLAG_ID(entry->id->flags) == MID_MACRO)
	{
	    if(MID_GET_FLAG_ISS(entry->id->flags) == 0)
	    {
	    	cur_def = def_find_by_id(gAdmMacros, NULL, entry->id);
	    }
	    else
	    {
	    	cur_def = def_find_by_id(gMgrVDB.macros, &(gMgrVDB.macros_mutex), entry->id);
	    }

	}

	/* Step 3: Print the TDC holding data for the entry. */
	ui_print_tdc(entry->contents, cur_def);
    printf("\n");

    /* print to RPC */
    char tmp_buf[D_VARENTRYSIZE];
    char *name = names_get_name(entry->id);
    size_t size;

    size = netui_print_tdc_def(entry->contents, cur_def, tmp_buf);
    tmp_buf[size] = '\0';

	AddVariableToQueue(name, TYPE_STRING, tmp_buf, receiver, 0, ts);
	SRELEASE(name);

    if(del_def)
    {
    	def_release_gen(cur_def);
    }
    return;
}

/******************************************************************************XXX
 *
 * \par Function Name: ui_print_reports
 *
 * \par Print all reports in the received reports queue.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/

void netui_print_reports(agent_t* agent)
{
	 LystElt report_elt;
	 //LystElt previousReportElt;
	 LystElt entry_elt;
	 rpt_t *cur_report = NULL;
	 rpt_entry_t *cur_entry = NULL;

	 if(agent == NULL)
	 {
		 AMP_DEBUG_ENTRY("netui_print_reports","(NULL)", NULL);
		 AMP_DEBUG_ERR("netui_print_reports", "No agent specified", NULL);
		 AMP_DEBUG_EXIT("netui_print_reports", "->.", NULL);
		 return;

	 }
	 AMP_DEBUG_ENTRY("netui_print_reports","(%s)", agent->agent_eid.name);

	 if(lyst_length(agent->reports) == 0)
	 {
		 AMP_DEBUG_ALWAYS("netui_print_reports","[No reports received from this agent.]", NULL);
		 AMP_DEBUG_EXIT("netui_print_reports", "->.", NULL);
		 return;
	 }

	 /* Free any reports left in the reports list. */
	 for (report_elt = lyst_first(agent->reports); report_elt; report_elt = lyst_next(report_elt))
	 {
		 /* Grab the current report */
	     if((cur_report = (rpt_t*)lyst_data(report_elt)) == NULL)
	     {
	        AMP_DEBUG_ERR("netui_print_reports","Unable to get report from lyst!", NULL);
	     }
	     else
	     {
	    	 uvast mid_sizes = 0;
	    	 uvast data_sizes = 0;

			/* Print the Report Header */
	    	//AddVariableToQueue("SentTo",TYPE_STRING,cur_report->recipient.name,&agent->agent_eid,strlen(cur_report->recipient.name),cur_report->time);
	    	//AddVariableToQueue("NumMids",TYPE_UINT32,lyst_length(cur_report->reports),agent->agent_eid.name);

	    	 printf("\n----------------------------------------");
	    	 printf("\n            DTNMP DATA REPORT           ");
	    	 printf("\n----------------------------------------");
	    	 printf("\nSent to   : %s", cur_report->recipient.name);
	    	 printf("\nTimestamp : %s", ctime(&(cur_report->time)));
	    	 printf("\n# Entries : %lu",
			 (unsigned long) lyst_length(cur_report->entries));
	    	 printf("\n----------------------------------------\n");

	    	 /* For each MID in this report, print and deleteit. */
	    	 for(entry_elt = lyst_first(cur_report->entries); entry_elt; entry_elt = lyst_next(entry_elt))
	    	 {
	    		 cur_entry = (rpt_entry_t*)lyst_data(entry_elt);

	    		 netui_print_entry(cur_entry, &cur_report->recipient, cur_report->time, &mid_sizes, &data_sizes);
	    	 }
	    	 printf("\n----------------------------------------\n");
	    	 printf("STATISTICS:\n");
	    	 printf("MIDs total "UVAST_FIELDSPEC" bytes\n", mid_sizes);
	    	 printf("Data total: "UVAST_FIELDSPEC" bytes\n", data_sizes);

	    	 if (mid_sizes + data_sizes > 0)
             {
                printf("Efficiency: %.2f%%\n", (double)(((double)data_sizes)/((double)mid_sizes + data_sizes)) * (double)100.0);
	    	 }

	    	 printf("----------------------------------------\n\n\n");
	     }
	 }
}



/******************************************************************************
 *
 * \par Function Name: ui_run_tests
 *
 * \par Run local manager tests to test out libraries.
 *
 * \par Notes:
 * \todo Move this to a test file.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *  06/25/13  E. Birrane     Removed references to priority field.
 *****************************************************************************/

void ui_run_tests()
{
	char *str;
	unsigned char *msg;

	/* Test 1: Construct an OID and serialize/deserialize it. */
	// # bytes (SDNV), followe dby the bytes.
	fprintf(stderr,"OID TEST 1\n----------------------------------------\n");
	unsigned char tmp_oid[8] = {0x07,0x01,0x02,0x03,0x04,0x05,0x06,0x00};
	uint32_t bytes = 0;

	fprintf(stderr,"Initial is ");
	utils_print_hex(tmp_oid,8);

	oid_t oid = oid_deserialize_full(tmp_oid, 8, &bytes);

	fprintf(stderr,"Deserialized %d bytes into:\n", bytes);
	str = oid_pretty_print(oid);
	fprintf(stderr,"%s",str);
	SRELEASE(str);

	msg = oid_serialize(oid,&bytes);
	fprintf(stderr,"Serialized %d bytes into ", bytes);
	utils_print_hex(msg,bytes);
	SRELEASE(msg);
	fprintf(stderr,"\n----------------------------------------\n");


	/* Test 2: Construct a MID and serialize/deserialize it. */
	fprintf(stderr,"MID TEST 1\n");
	uvast issuer = 0, tag = 0;

	mid_t *mid = mid_construct(0,NULL, NULL, oid);
	msg = (unsigned char*)mid_to_string(mid);
	fprintf(stderr,"Constructed mid: %s\n", msg);
	SRELEASE(msg);

	msg = mid_serialize(mid, &bytes);
	fprintf(stderr,"Serialized %d bytes into ", bytes);
	utils_print_hex(msg, bytes);

	uint32_t b2;
	mid_t *mid2 = mid_deserialize(msg, bytes, &b2);
	SRELEASE(msg);
	msg = (unsigned char *)mid_to_string(mid2);

	fprintf(stderr,"Deserialized %d bytes into MID %s\n", b2, msg);
	SRELEASE(msg);
	mid_release(mid2);
	mid_release(mid);
}



void *netui_thread(int *running)
{
	AMP_DEBUG_ENTRY("ui_thread","(0x%x)", (unsigned long) running);

	ui_eventLoop(running);

	AMP_DEBUG_ALWAYS("ui_thread","Exiting.", NULL);

	AMP_DEBUG_EXIT("ui_thread","->.", NULL);

#ifdef HAVE_MYSQL
	db_mgt_close();
#endif


	pthread_exit(NULL);

	return NULL;
}

/******************************************************************************
 *
 * \par Function Name: netui_build_command
 *
 * \par Purpose: The core of the lazyRPC system, this function takes a buffer, and creates a cmdFormat.
 * \return 1 if successful, 0 if failed
 *
 *
 * \param[in]   inBuffer	A pointer to the null-terminated text buffer.
 * \param[out]  cmdFormat	The processed command
 * \param[in]   bufSize	The size of the buffer
 *
 * \par Notes: This function modifies the inBuffer, placing it at the end of the processed RPC command.
 *				As long as the return value != 0, keep calling this function with the same parameters in
 *				order to receive a new command.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

short netui_build_command(char** inBuffer,cmdFormat* cmdOutput,size_t bufSize)
{
	//Sanity check
	if(cmdOutput==NULL)
		return 0;

	char* curTok;
	char* buffer = * inBuffer;
	char* tok_r;
	unsigned char cmdIdx = 0;
	char* retPtr = NULL;
	//char* intBuffer=(char*)STAKE(D_INPUTBUFFERSIZE);
	char* inputBuffer;//=intBuffer;
	char* curTokEnd;
	size_t tokSize;

	cmdOutput->cmdChunks=(char**)STAKE(D_INPUTMAXCHUNKS*sizeof(char*));
	memset(cmdOutput->cmdChunks,0,D_INPUTMAXCHUNKS*(sizeof(char*)));
	//Copy string to internal storage
	//strncpy(inputBuffer,buffer,bufSize);
	curTokEnd = strpbrk(buffer,";");

	if(curTokEnd!=NULL) /*& (curTokEnd-buffer > bufSize))*/
	{
		curTokEnd[0]='\0';
		*inBuffer = curTokEnd+1;

	}
	else
	{
		AMP_DEBUG_ERR("Netui_build_command","Invalid command",NULL);
		return 0;
	}
	//AMP_DEBUG_INFO("build","%s",buffer);
	char* argPtr=strchr(buffer,'=');
	if(argPtr != NULL)
	{
	    size_t argLen = strlen(argPtr);

		cmdOutput->arguments=(char*)STAKE(argLen+1);
		memset(cmdOutput->arguments,'\0',argLen+1);
		strncpy(cmdOutput->arguments,argPtr+1,argLen);
	}
	//Find acting UID
	cmdOutput->eid=strtok(buffer,"\\");

	//AMP_DEBUG_INFO("netui_build_command, found EID","%s",cmdOutput->eid)
	if(cmdOutput->eid==NULL)
	{
		//SRELEASE(inputBuffer);
		return 0;
	}
	inputBuffer=strtok(NULL,"\\=");

	for(;;inputBuffer=NULL)
	{
		AMP_DEBUG_ERR("netui_build_command","loop",NULL);
		curTok=strtok_r(inputBuffer,".",&tok_r);

		if(curTok==NULL)
		{
			if(cmdIdx>0)
				break;
			else
			{
				//SRELEASE(intBuffer);
				return 0;
			}

		}
		AMP_DEBUG_ERR("netui_build_command","loop3",NULL);
	//Now, do the parsing

		tokSize = strlen(curTok);
		if(tokSize==0)
		{
			AMP_DEBUG_ERR("netui_build_command","Undefined string, continuing",NULL);
			continue;
		}

		cmdOutput->cmdChunks[cmdIdx] = (char*)STAKE(tokSize);
		sscanf(curTok,"%s",cmdOutput->cmdChunks[cmdIdx]);
		AMP_DEBUG_INFO("Netui_build_command","Copied token (%d): %s",cmdIdx,cmdOutput->cmdChunks[cmdIdx]);
		cmdIdx++;

	}
	cmdOutput->numChunks=cmdIdx-1;
	SRELEASE(inputBuffer);
	return 1;
}
/******************************************************************************
 *
 * \par Function Name: netui_create_cmdformat
 *
 * \par Purpose: allocates memory for a cmdFormat
 * \return a pointer to the new cmdFormat entry
 *
 *
 * \par Notes: It's up to the user to free this.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

cmdFormat* netui_create_cmdformat()
{
	cmdFormat* outFmt = (cmdFormat*)STAKE(sizeof(cmdFormat));
	outFmt->numChunks=0;
	outFmt->arguments=NULL;
	return outFmt;
}

/******************************************************************************
 *
 * \par Function Name: netui_free_cmdformat
 *
 * \par Purpose: Frees an allocated cmdFormat entry
 * \return none
 *
 *
 * \param[in]   toFree		A pointer to the cmdFormat struct to be freed
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
void netui_free_cmdformat(cmdFormat* toFree)
{
	if(toFree == NULL)
		return;
	unsigned int x;
	AMP_DEBUG_ENTRY("netui_free_cmdformat","Freeing %#u",toFree);
	for(x=0;x<=toFree->numChunks;x++)
	{
		if(toFree->cmdChunks[x]!=NULL)
			SRELEASE(toFree->cmdChunks[x]);
	}
	if(toFree->arguments!=NULL)
		SRELEASE(toFree->arguments);
	SRELEASE(toFree);
}
/******************************************************************************
 *
 * \par Function Name: netui_find_agent_by_name
 *
 * \par Purpose: Performs a case-insensitive search for an given agent, and returns
 * 				an agent_t
 * \return an agent_t*, or null if no agent is found
 *
 *
 * \param[in]   name		The null-terminated name to search.
 * \par Notes: This exists for the RPC system
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
agent_t* netui_find_agent_by_name(char* name)
{
	int i = 1;
	LystElt element;

	AMP_DEBUG_ENTRY("netui_find_agent_by_name",NULL,NULL);

	AMP_DEBUG_INFO("netui_find_agent_by_name", "looking for: %s",name);

	element = lyst_first(known_agents);
	if(element == NULL)
	{
	  return NULL;
	}
	while(element != NULL)
	{
	  if(strcmp((char *) lyst_data(element),name)==0)
	  {
	  	AMP_DEBUG_INFO("netui_find_agent_by_name", "found element %s", name);
	  	return (agent_t *) lyst_data(element);
	  }
	  element = lyst_next(element);
	}

	return NULL;
}
/******************************************************************************
 *
 * \par Function Name: netui_find_ctrl_idx_by_name
 *
 * \par Purpose: Performs a case-insensitive search for an given control name, and returns
 * 				the index
 * \return the index, or 0 if none found
 *
 *
 * \param[in]   name		The null-terminated name to search.
 * \par Notes: This exists for the RPC system
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
int netui_find_ctrl_idx_by_name(char* name)
{
	int i = 0;
	LystElt elt = 0;
	mgr_name_t *cur = NULL;

	AMP_DEBUG_ENTRY("netui_find_ctrl_idx_by_name","(%s)",name);

	AMP_DEBUG_INFO("netui_find_ctrl_idx_by_name","%s.", name);


	Lyst result = names_retrieve(ADM_ALL, MID_CONTROL);
	for(elt = lyst_first(result); elt; elt = lyst_next(elt))
	{
        cur = (mgr_name_t *) lyst_data(elt);
		if(strcasecmp(cur->name,name)==0) {
            lyst_destroy(result);
            return i; //Found it!
		}
        i++;
	}

	lyst_destroy(result);

	AMP_DEBUG_INFO("netui_find_ctrl_idx_by_name","finding %s failed.", name);
	return -1;
}


/******************************************************************************
 *
 * \par Function Name: netui_find_data_idx_by_name
 *
 * \par Purpose: Performs a case-insensitive search for an given data reference,
 * 				 and returns the index
 * \return the index, or 0 if none found
 *
 *
 * \param[in]   name		The null-terminated name to search.
 * \par Notes: This exists for the RPC system
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
int netui_find_data_idx_by_name(char* name)
{
	int i = 0;
	LystElt elt = 0;
	mgr_name_t *cur = NULL;

	AMP_DEBUG_ENTRY("netui_find_data_idx_by_name","(%s)",name);

	AMP_DEBUG_INFO("netui_find_data_idx_by_name","%s.", name);

	Lyst result = names_retrieve(ADM_ALL, MID_ATOMIC);  // XXX: MID_COMPUTED and MID_REPORT also required here?
	for(elt = lyst_first(result); elt; elt = lyst_next(elt))
	{
		cur = (mgr_name_t*) lyst_data(elt);
		if(strcasecmp(cur->name,name)==0)
			return i; //Found it!
        i++;
	}

	AMP_DEBUG_INFO("netui_find_data_idx_by_name","finding %s failed.", name);
	return -1;
}

/******************************************************************************XXX
 *
 * \par Function Name: netui_define_raw_mid_params
 *
 * \par Purpose: Given a null-terminated mid, as well as a null-terminated argument,
 *				 converts it into a mid_t structure. This will parse datalists, and put
 *				 them into the mid_t. It's also written to allow general arrays, but that
 *				 functionality is currently unimplemented.
 * \return
 *
 *
 * \param[in]   name	A pointer to the null-terminated text buffer containing the mid
 * \param[in]   arguments A pointer to the null-terminated text buffer containing all args
 * \param[in]   num_parms the number of arguments which this MID takes
 * \param[out]  mid		The mid_t structure, or null if a failure occurs
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
void netui_define_raw_mid_params(char *name, char* arguments,int num_parms, mid_t *mid)
{
	char mid_str[256];
	char line[256];
	char* paramBuffer;
	int len = 0;
	int i = 0;
	uint32_t size = 0;
	char* argBuf;
	char* curArg;
	char* arg_r;
	char** allArgs;//=(char**)malloc(500*sizeof(char*));
	uint8_t numArgs=0;

	AMP_DEBUG_ENTRY("netui_define_raw_mid_params", "(0x%x, %d, 0x%x)",
			          (unsigned long) name, num_parms, (unsigned long) mid);

	if((name == NULL) || (mid == NULL))
	{
		AMP_DEBUG_ERR("netui_define_mid_params", "Bad Args.", NULL);
		AMP_DEBUG_EXIT("netui_define_mid_params","->.", NULL);
		return;
	}

//	Parse arguments
	allArgs = netui_parse_arguments(arguments,&numArgs);
	if(numArgs<num_parms)
	{
		AMP_DEBUG_WARN("Not enough parameters","",NULL);
		free(allArgs);
		return;
	}
	for(i = 0; i < num_parms; i++)
	{

		//If the string is opened with a {, then its an array... We don't really care (we pass it as a single string), except in the cases where it is an array of datalists
		if(strspn(allArgs[i],"{[")==2)
		{
			AMP_DEBUG_INFO("netui_define_raw_mid_params","In datalist processor",NULL);
			//char* arrayReentry;
			Lyst dlDatacol = lyst_create();
			blob_t* datacolEntry;
			uint8_t numDatalists;
			char* arrayStart = strchr(allArgs[i],'{');

			if(arrayStart==NULL)
			{
				AMP_DEBUG_ERR("netui_define_raw_mid_params","Couldn't find array start",NULL);
				lyst_destroy(dlDatacol);
				break; //Keep going with what we have
			}
			arrayStart++;
			char* arrayEnd = strchr(arrayStart,'}');
			if(arrayEnd==NULL)
			{
				AMP_DEBUG_ERR("netui_define_raw_mid_params","Couldn't find array end %s",arrayStart);
				lyst_destroy(dlDatacol);
				break; //Keep going with what we have
			}
			//Delimit array
			arrayEnd[0]='\0';

			char** datalists = netui_parse_arguments(arrayStart,&numDatalists);

			AMP_DEBUG_INFO("netui_define_mid_params","Found # datalists: %d in %s",numDatalists,arrayStart);
			for(unsigned int x = 0; x < numDatalists ; x++)
			{
				char* arrayTok = datalists[x]+1;

				char* arrayEndBracket = strchr(arrayTok,']');
				if(arrayEndBracket)
					arrayEndBracket[0]='\0';
				else
				{
					AMP_DEBUG_INFO("netui_define_mid_params","Failed to parse, continuing",NULL);

					continue;
				}
				AMP_DEBUG_INFO("netui_define_mid_params","Found datalist: %s",arrayTok);

				tdc_t* curDl = netui_parse_tdc(arrayTok);
				//datacolE7ntry = (datacol_entry_t*)STAKE(sizeof(datacol_entry_t));

				AMP_DEBUG_INFO("netui_define_mid_params","Serializing %s",arrayTok);

                // XXX: check if this still serializes as before.
				datacolEntry->value = tdc_serialize(curDl, &(datacolEntry->length));
				lyst_insert_last(dlDatacol,datacolEntry);
				tdc_destroy(&curDl);
				AMP_DEBUG_INFO("netui_define_mid_params","serializing done",NULL);
			}
			//Now, serialize the datacol
			AMP_DEBUG_INFO("netui_define_mid_params","Performing final serialization %d",size);
			paramBuffer=(char*)dc_serialize(dlDatacol,&size);
			AMP_DEBUG_INFO("netui_define_mid_params","Serialized %d bytes to datacol",size);
			dc_destroy(&dlDatacol);
		}
		else
		{
			paramBuffer=allArgs[i];
			size=strlen(allArgs[i]);
		}
		AMP_DEBUG_INFO("netui_define_mid_params","copying arg: %d of size %d",i,size);
    	//XXX mid_add_param(mid, (unsigned char*)paramBuffer, size);
    	AMP_DEBUG_INFO("netui_define_mid_params","mid_add_param STILL TO FIX!!!", NULL);
	}

	AMP_DEBUG_EXIT("ui_define_mid_params","->.", NULL);
}


void netui_define_mid_params(cmdFormat* curCmd,char *name, int num_parms, mid_t *mid)
{
	AMP_DEBUG_ENTRY("netui_define_mid_params", "(0x%x, %d, 0x%x)",
			          (unsigned long) name, num_parms, (unsigned long) mid);

	netui_define_raw_mid_params(name,curCmd->arguments,num_parms,mid);
	AMP_DEBUG_EXIT("ui_define_mid_params","->.", NULL);
}

/******************************************************************************XXX
 *
 * \par Function Name: netui_parse_single_mid_stringl
 *
 * \par Purpose: Given a string containing a single mid, along with the arguments,
 *				 create a mid_t, and append it to the lyst
 * \return
 *
 * \par Notes:  This is here so that we can provide parameters to non-control MID's (eg: for reporting)
 *				which are provided in an array
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
void netui_parse_single_mid_str(Lyst mids,char *mid_str, char* arguments,int max_idx, int type)
{
	char *cur_mid = NULL;
	char *tmp = NULL;
	int cur_mid_idx = 0;
	uint32_t mid_size = 0;

	AMP_DEBUG_ENTRY("netui_parse_single_mid_str","(0x%x) %s",mid_str,mid_str);

	/* Step 0: Sanity Check. */
	if(mid_str == NULL)
	{
		AMP_DEBUG_ERR("netui_parse_single_mid_str","Bad args.", NULL);
		AMP_DEBUG_EXIT("netui_parse_single_mid_str","->NULL.", NULL);
		return;
	}


	AMP_DEBUG_INFO("netui_parse_single_mid_str","Read MID index of %s", mid_str);

	/* Step 1a. Convert and check MID index. */
	if((cur_mid_idx = atoi(mid_str)) <= max_idx)
	{
		uint32_t bytes = 0;
		mid_t *midp = NULL;
		char *name = NULL;
		int num_parms = 0;
		int mid_len = 0;

		/* Step 1b: Grab MID and put it into the list. */

		switch(type)
		{
		case MID_ATOMIC:
		case MID_COMPUTED:
		case MID_REPORT:
			{
				adm_datadef_t *cur = adm_find_datadef_by_idx(cur_mid_idx);
				midp = mid_copy(cur->mid);
				num_parms = cur->num_parms;
				/***
				name = adus[cur_mid_idx].name;
				num_parms = adus[cur_mid_idx].num_parms;
				mid_len = adus[cur_mid_idx].mid_len;
				midp = mid_deserialize((unsigned char*)&(adus[cur_mid_idx].mid),
									adus[cur_mid_idx].mid_len,&bytes);
				 */
			}
			break;
		case MID_CONTROL:
			{
				adm_ctrl_t *cur = adm_find_ctrl_by_idx(cur_mid_idx);
				midp = mid_copy(cur->mid);
				num_parms = cur->num_parms;

				AMP_DEBUG_INFO("netui_parse_single_mid_str","Entered MID_TYPE_CONTROL",NULL);

			/*	name = ctrls[cur_mid_idx].name;
				num_parms = ctrls[cur_mid_idx].num_parms;
				mid_len = ctrls[cur_mid_idx].mid_len;
				midp = mid_deserialize((unsigned char*)&(ctrls[cur_mid_idx].mid),
								   ctrls[cur_mid_idx].mid_len,&bytes);
			*/
			//	midp = mid_deserialize((unsigned char*)midp->raw,mid_len,&bytes);
			}
			break;
		case MID_LITERAL:
			/* \todo: Write this. */
		case MID_OPERATOR:
			/* \todo: Write this. */
		default:
			AMP_DEBUG_ERR("ui_parse_mid_str","Unknown type %d", type);
			AMP_DEBUG_EXIT("ui_parse_mid_str","->NULL.",NULL);
			lyst_destroy(mids);
			return;
		}

		/* If this MID has parameters, get them */
		if(num_parms > 0)
		{
		    AMP_DEBUG_INFO("netui_parse_single_mid_str","Get MID params: %d", num_parms);
			netui_define_raw_mid_params(mid_str,arguments, num_parms, midp);
			mid_internal_serialize(midp);
		}

		mid_size += mid_len;
		lyst_insert_last(mids, midp);
	}
	else
	{
		AMP_DEBUG_ERR("ui_parse_mid_str",
						"Bad MID index: %d. Max is %d. Skipping.",
						cur_mid_idx, max_idx);
	}


	AMP_DEBUG_EXIT("ui_parse_mid_str","->0x%x.", mids);
}
/******************************************************************************
 *
 * \par Function Name: netui_parse_single_mid_string
 *
 * \par Purpose: Given a cmdFormat containing multiple mids, along with the arguments,
 *				 create a mid_t, and append it to the lyst
 * \return
 *
 * \par Notes:  This is here mostly for non-parameterized MID arrays
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
Lyst netui_parse_mid_str(cmdFormat* curCmd, char *mid_str, int max_idx, int type)
{
	Lyst mids;
	char* cur_mid;
	char* tmp;
	mids=lyst_create();

	for(cur_mid = strtok_r(mid_str, ", ", &tmp);
	    cur_mid != NULL;
	    cur_mid = strtok_r(NULL, ", ", &tmp))
	{
		netui_parse_single_mid_str(mids,mid_str, curCmd->arguments,max_idx,type);
	}

	return mids;
}
/******************************************************************************
 *
 * \par Function Name: netui_parse_arguments
 *
 * \par Purpose: Given a null-terminated string containing arguments, create an
 * 				 array of tokens, being careful to maintain arrays as single strings
 *				 for datalist parsing.
 * \return dynamic array of argument tokens
 *
 *
 * \param[in]   argString		The null-terminated string containing args
 * \param[out]  numArgsOut		The number of arguments found
 * \par Notes: This exists for the RPC system. In addition, the user MUST free the returned value
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
char** netui_parse_arguments(char* argString,uint8_t* numArgsOut)
{
	char* argBuf;
	char* curArg;
	char* arg_r;
	char* cursor;
	AMP_DEBUG_INFO("netui_parse_arguments","Working with argument: %s",argString);
	char** allArgs=(char**)STAKE(D_INPUTMAXCHUNKS*sizeof(char*));
	unsigned int numArgs=0;

	cursor=argString;
	for(argBuf=argString;cursor!=NULL;)
	{
		/*************
		Check for datalist or array
			Iterate to end of DL or array
		find next comma
		*************/
		//printf("new argString = %s\n",cursor);
		if((cursor[0]=='{') || (cursor[0]=='[') || (cursor[0]=='\"'))//We have one
		{

			argBuf=cursor;
			char endChar;

			switch(cursor[0])
			{
				case '{':endChar='}';break;
				case '[':endChar=']';break;
				case '\"':endChar='\"';break;
			}
			cursor = strchr(cursor+1,endChar);

			if(cursor==NULL)
			{
				break;
			}
			else
				cursor++;

		}
		else
			cursor = strchr(cursor,',');

		if(cursor==NULL)
		{
			break;
		}
		curArg = (char*)STAKE((cursor-argBuf));

		strncpy(curArg,argBuf,cursor-argBuf);
		curArg[cursor-argBuf]='\0';
//		cursor = argBuf;
		AMP_DEBUG_INFO("blah","copying arg: %s of size %d",curArg,cursor-argBuf);
		cursor++;
		argBuf=cursor;
		allArgs[numArgs]=curArg;
		numArgs++;



	}
	*numArgsOut=numArgs;
	return allArgs;
}

/******************************************************************************
 *
 * \par Function Name: get_num_reports_by_agent
 *
 * \par Purpose: creates variable queue entries containing # reports/agent
 * \par Notes: This exists for the RPC system
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
void netui_get_num_reports_by_agent()
{
	AMP_DEBUG_INFO("netui_get_num_reports_by_agent","Starting",NULL);

	LystElt elt;
	uint32_t tempValue;
	lockResource(&agents_mutex);

	for(elt = lyst_first(known_agents);elt;elt=lyst_next(elt))
	{
		AMP_DEBUG_INFO("netui_get_num_reports_by_agent","in",NULL);
		agent_t* agent = (agent_t*)lyst_data(elt);

		lockResource(&agent->mutex);
		tempValue = lyst_length(agent->reports);
		unlockResource(&agent->mutex);

		AddVariableToQueue("num_reports",TYPE_UINT32,&tempValue,&agent->agent_eid, 0, 0);

	}

	unlockResource(&agents_mutex);
	AMP_DEBUG_EXIT("netui_get_num_reports_by_agent","ending",NULL);
}

void ui_list_adms()
{

}

void ui_list_atomic()
{
	ui_list_gen(ADM_ALL, MID_ATOMIC);
}

void ui_list_compdef()
{
	ui_list_gen(ADM_ALL, MID_COMPUTED);
}

void ui_list_ctrls()
{
	ui_list_gen(ADM_ALL, MID_CONTROL);
}

mid_t * ui_get_mid(int adm_type, int mid_id, uint32_t opt)
{
	mid_t *result = NULL;

	int i = 0;
	LystElt elt = 0;
	mgr_name_t *cur = NULL;

	AMP_DEBUG_ENTRY("ui_print","(%d, %d)",adm_type, mid_id);

	Lyst names = names_retrieve(adm_type, mid_id);

	for(elt = lyst_first(names); elt; elt = lyst_next(elt))
	{
		if(i == opt)
		{
			cur = (mgr_name_t *) lyst_data(elt);
			result = mid_copy(cur->mid);
			break;
		}
		i++;
	}

	lyst_destroy(names);
	AMP_DEBUG_EXIT("ui_print","->.", NULL);

	return result;
}


void ui_list_gen(int adm_type, int mid_id)
{
	  int i = 0;
	  LystElt elt = 0;
	  mgr_name_t *cur = NULL;

	  AMP_DEBUG_ENTRY("ui_print","(%d, %d)",adm_type, mid_id);

	  Lyst result = names_retrieve(adm_type, mid_id);

	  for(elt = lyst_first(result); elt; elt = lyst_next(elt))
	  {
		  cur = (mgr_name_t *) lyst_data(elt);
		  printf("%3d) %-50s - %-25s\n", i, cur->name, cur->descr);
		  i++;
	  }

	  lyst_destroy(result);
	  AMP_DEBUG_EXIT("ui_print","->.", NULL);
}

void ui_list_literals()
{
	ui_list_gen(ADM_ALL, MID_LITERAL);
}

void ui_list_macros()
{
	ui_list_gen(ADM_ALL, MID_MACRO);
}

void ui_list_ops()
{
	ui_list_gen(ADM_ALL, MID_OPERATOR);
}

void ui_list_rpts()
{
	ui_list_gen(ADM_ALL, MID_REPORT);
}


/******************************************************************************
 *
 * \par Function Name: netui_parse_tdc
 *
 * \par Purpose:given a string containing a datalist, create a tdc (with
 * 				intact typing)
 * \return the tdc
 *
 *
 * \param[in]   dlText		The null-terminated string containing the datalist
 * \par Notes: This exists for the RPC system, and will attempt to fail
 *				gracefully. The user must verify the length of the returned
 *				tdc, or hope that the client validates.
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
tdc_t* netui_parse_tdc(char* dlText)
{
	AMP_DEBUG_INFO("netui_parse_tdc","Parsing %s",dlText);
	tdc_t* outDl = tdc_create(NULL, NULL, 0);
	char* strReentry;
	char* valueTxt=(char*)STAKE(64);
	char* typeTxt=(char*)STAKE(64);
	size_t valueSize;
	for(char* curItem = strtok_r(dlText,",",&strReentry) ; curItem ; curItem = strtok_r(NULL,",",&strReentry))
	{
		//Parse the textual form
		memset(valueTxt,'\0',64);
		sscanf(curItem,"(%[^)]) %64c",typeTxt,valueTxt);

		//Determine type
		amp_type_e type=type_from_str(typeTxt);


		if(type==AMP_TYPE_STRING)
		{
			valueSize=strlen(valueTxt);
			//We need to seek to the end of the entry, if it's quoted
		}
		else
			valueSize=type_get_size(type);

		if(valueSize==0)
		{
			AMP_DEBUG_ERR("netui_parse_tdc","Couldn't determine type, continuing...",NULL);
			continue;
		}

		//Create a temporary buffer
		char* value = (char*)STAKE(valueSize);
		memset(value,0,valueSize);
		AMP_DEBUG_INFO("netui_parse_tdc","Using specifier: \"%s\" for data \"%s\"",type_get_fieldspec(type),valueTxt);
		sscanf(valueTxt,type_get_fieldspec(type),value);

		if(tdc_insert(outDl,type,value,valueSize)==AMP_TYPE_UNK)
		{
			AMP_DEBUG_ERR("netui_parse_tdc","Failed to insert, exiting",NULL);
			SRELEASE(value);

			break;
		}
		//Free stuff
		SRELEASE(value);

	}
	SRELEASE(valueTxt);
	SRELEASE(typeTxt);

	return outDl;
}

size_t netui_print_tdc(void* inBuffer,size_t size,char* outBuffer)
{
	uint32_t bytesUsed;
	uint32_t bytesUsedPerDL;
	Lyst datalists = dc_deserialize((uint8_t*)inBuffer,size,&bytesUsed);
	uint8_t* data;
	size_t dlEltSize;
	char* cursor = &outBuffer[0];
	unsigned int numBytesPrinted=0;
	//Start the datalist textual form
	(cursor++)[0]='{';
	//This line also adds the appending comma between datalists.
	for(LystElt dlElt = lyst_first(datalists) ; dlElt ; dlElt = lyst_next(dlElt),(cursor++)[0]=',')
	{
		AMP_DEBUG_INFO("netui_print_tdc","Getting tdc...",NULL);
		blob_t* dataCol = (blob_t*)lyst_data(dlElt);
		tdc_t* curDl = tdc_deserialize(dataCol->value,dataCol->length,&bytesUsedPerDL);

		//Alright, now we have a single DL
		netui_print_tdc_def(curDl, NULL, cursor);

		tdc_destroy(&curDl);
	}

	(cursor++)[0]='}';
	(cursor++)[0]='\0';

	return cursor-outBuffer;
}

size_t netui_print_tdc_def(tdc_t* tdc, def_gen_t* cur_def, char* outBuffer)
{
	LystElt elt = NULL;
	LystElt def_elt = NULL;
	uint32_t i = 0;
	amp_type_e cur_type;
	blob_t *cur_entry = NULL;
	value_t *cur_val = NULL;
	char *start = outBuffer;

	if(tdc == NULL)
	{
		AMP_DEBUG_ERR("netui_print_tdc","Bad Args.", NULL);
		return 0;
	}

	if(cur_def != NULL)
	{
		if(lyst_length(cur_def->contents) != tdc->hdr.length)
		{
			AMP_DEBUG_WARN("netui_print_tdc","def and TDC length mismatch: %d != %d. Ignoring.",
					        lyst_length(cur_def->contents), tdc->hdr.length);
			cur_def = NULL;
		}
	}


	elt = lyst_first(tdc->datacol);
	if(cur_def != NULL)
	{
		def_elt = lyst_first(cur_def->contents);
	}

    (outBuffer++)[0]='[';

	for(i = 0; ((i < tdc->hdr.length) && elt); i++)
	{
		cur_type = (amp_type_e) tdc->hdr.data[i];

		//printf("\n\t");
		if(cur_def != NULL)
		{
			printf("Value %d (", i);
			ui_print_mid((mid_t *) lyst_data(def_elt));
			printf(") ");
		}

		// \todo: Check return values.
		if((cur_entry = lyst_data(elt)) == NULL)
		{
			outBuffer+= netui_print_string("NULL",4,outBuffer);
		}
		else
		{
			//ui_print_val(cur_type, cur_entry->value, cur_entry->length);
			outBuffer+= sprintf(outBuffer,"(%s) ",type_to_str(cur_type));

            switch(cur_type)
            {
                case AMP_TYPE_STRING:
                    outBuffer+= netui_print_string(cur_entry->value,cur_entry->length,outBuffer);
                break;
                case AMP_TYPE_UINT:
                    outBuffer+= netui_print_uint32(cur_entry->value,cur_entry->length,outBuffer);
                break;
                case AMP_TYPE_INT:
                    outBuffer+= netui_print_int32(cur_entry->value,cur_entry->length,outBuffer);
                break;
                case AMP_TYPE_REAL32:
                    outBuffer+= netui_print_real32(cur_entry->value,cur_entry->length,outBuffer);
                break;
                case AMP_TYPE_REAL64:
                    outBuffer+= netui_print_real64(cur_entry->value,cur_entry->length,outBuffer);
                break;
                case AMP_TYPE_VAST:
                    outBuffer+= netui_print_vast(cur_entry->value,cur_entry->length,outBuffer);
                break;
                case AMP_TYPE_UVAST:
                    outBuffer+= netui_print_uvast(cur_entry->value,cur_entry->length,outBuffer);
                break;
                default:
                    AMP_DEBUG_ERR("netui_print_tdc","Type not known",NULL);
            }
		}

		elt = lyst_next(elt);

		if (elt)
            (outBuffer++)[0]=',';

		if(cur_def != NULL)
		{
			def_elt = lyst_next(def_elt);
		}
	}

    (outBuffer++)[0]=']';

    return (size_t)(outBuffer - start);
}

size_t netui_print_string(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%s",(char*)inBuffer);
}
size_t netui_print_uint32(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%u",*(uint32_t*)inBuffer);
}
size_t netui_print_int32(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%d",*(int32_t*)inBuffer);
}

size_t netui_print_uint64(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%lu",*(uint64_t*)inBuffer);
}

size_t netui_print_int64(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%ld",*(int64_t*)inBuffer);
}
size_t netui_print_real32(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%f",*(float*)inBuffer);
}
size_t netui_print_real64(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,"%f",*(double*)inBuffer);
}
size_t netui_print_vast(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,VAST_FIELDSPEC,*(vast*)inBuffer);
}
size_t netui_print_uvast(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,UVAST_FIELDSPEC,*(uvast*)inBuffer);
}
