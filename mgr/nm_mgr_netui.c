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
#include "shared/utils/utils.h"
#include "shared/adm/adm.h"
#include "shared/primitives/rules.h"
#include "shared/primitives/mid.h"
#include "shared/primitives/oid.h"
#include "shared/primitives/datalist.h"
#include "shared/msg/pdu.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>


/*
#define UI_MAIN_MENU 0
#define UI_DEF_MENU  1
#define UI_CTRL_MENU 2
#define UI_RPT_MENU  3
*/

int gContext;


/******************************************************************************
 *
 * \par Function Name: ui_build_mid
 *
 * \par Builds a MID object from user input.
 *
 * \par Notes:
 *
 * \retval NULL  - Error
 * 		   !NULL - The constructed MID.
 *
 * \param[in]  mid_str  The user input to define the MID.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/

//mid_t *ui_build_mid(char *mid_str)
//{
//	mid_t *result = NULL;
//	uint8_t *tmp = NULL;
//	uint32_t len = 0;
//	uint32_t bytes = 0;
//	adm_datadef_t adu;
//
//	DTNMP_DEBUG_ENTRY("ui_build_mid","(0x%x)", mid_str);
//
//	/* Step 0: Sanity check. */
//	if(mid_str == NULL)
//	{
//		DTNMP_DEBUG_ERR("ui_build_mid","Bad args.", NULL);
//		DTNMP_DEBUG_EXIT("ui_build_mid","->NULL", NULL);
//		return NULL;
//	}
//
//	/* Step 1: Convert the string into a binary buffer. */
//    if((tmp = utils_string_to_hex((unsigned char*)mid_str, &len)) == NULL)
//    {
//    	DTNMP_DEBUG_ERR("ui_build_mid","Can't Parse MID ID of %s.", mid_str);
//		DTNMP_DEBUG_EXIT("ui_build_mid","->NULL", NULL);
//		return NULL;
//    }
//
//    /* Step 2: Build an ADU from the buffer. */
// //   memcpy(adu.mid, tmp, len);
// //   adu.mid_len = len;
////    SRELEASE(tmp);
//
//    /* Step 3: Build a mid by "deserializing" the ADU into a MID. */
////	result = mid_deserialize((unsigned char*)&(adu.mid),ADM_MID_ALLOC,&bytes);
//    result = mid_deserialize(tmp, len, &bytes);
//
//	DTNMP_DEBUG_EXIT("ui_build_mid","->0x%x", result);
//
//	return result;
//}

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
    	DTNMP_DEBUG_ENTRY("ui_clear_reports","(NULL)", NULL);
    	DTNMP_DEBUG_ERR("ui_clear_reports", "No agent specified.", NULL);
        DTNMP_DEBUG_EXIT("ui_clear_reports","->.",NULL);
        return;
    }
    DTNMP_DEBUG_ENTRY("ui_clear_reports","(%s)",agent->agent_eid.name);

	int num = lyst_length(agent->reports);
	rpt_clear_lyst(&(agent->reports), NULL, 0);
	g_reports_total -= num;

	DTNMP_DEBUG_ALWAYS("ui_clear_reports","Cleared %d reports.", num);
    DTNMP_DEBUG_EXIT("ui_clear_reports","->.",NULL);
}



/******************************************************************************
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
	Lyst mids = lyst_create();
	uint32_t size = 0;

	if(agent == NULL)
	{
		DTNMP_DEBUG_ERR("netui_construct_ctrl_by_idx", "No agent specified.", NULL);
		return;
	}
	DTNMP_DEBUG_INFO("netui_construct_ctrl_by_idx","(%s %d)", curCmd->eid,curCmd->numChunks);



	/* Step 1: Parse the user input. */
	char* midIdxC;
	int midIdx = netui_find_ctrl_idx_by_name((curCmd->cmdChunks[curCmd->numChunks]));

	if(midIdx==-1)
	{
		DTNMP_DEBUG_ERR("netui_construct_ctrl_by_idx","Couldn't find ADM %d",curCmd->numChunks);
		return;
	}
	sprintf(midIdxC,"%d",midIdx);
	//sscanf(curCmd->arguments,"%s", &offset, mid_str);
	mids = netui_parse_mid_str(curCmd,midIdxC, lyst_length(gAdmCtrls)-1, MID_TYPE_CONTROL);

	/* Step 2: Construct the control primitive. */
	ctrl_exec_t *entry = ctrl_create_exec(0, mids);

	/* Step 3: Construct a PDU to hold the primitive. */
	uint8_t *data = ctrl_serialize_exec(entry, &size);
	pdu_msg_t *pdu_msg = pdu_create_msg(MSG_TYPE_CTRL_EXEC, data, size, NULL);
	pdu_group_t *pdu_group = pdu_create_group(pdu_msg);

	/* Step 4: Send the PDU. */
	iif_send(&ion_ptr, pdu_group, agent->agent_eid.name);

	/* Step 5: Release remaining resources. */
	pdu_release_group(pdu_group);
	ctrl_release_exec(entry);

	DTNMP_DEBUG_EXIT("netui_construct_ctrl_by_idx","->.", NULL);
}



/******************************************************************************
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
		DTNMP_DEBUG_ENTRY("ui_construct_time_rule_by_idx","(NULL)", NULL);
		DTNMP_DEBUG_ERR("ui_construct_time_rule_by_idx", "Null EID", NULL);
		DTNMP_DEBUG_EXIT("ui_construct_time_rule_by_idx","->.", NULL);
		return;
	}
	DTNMP_DEBUG_INFO("ui_construct_time_rule_by_idx","(%s)", agent->agent_eid.name);

	/* Step 1: Read and parse the rule. */
	/* Step 1a: "tokenize" the string */

	char** args=netui_parse_arguments(curCmd->arguments,&numArgs);
	if(numArgs<4)
	{
		DTNMP_DEBUG_ERR("netui_construct_time_rule_by_idx","Not enough arguments %d",numArgs);
		return;
	}

	offset=atoi(args[0]);
	period=atoi(args[1]);
	evals=atoi(args[2]);

	/* Step 1b: Split the MID array */
	unsigned int x = 3;

	for(x=3;x<numArgs;x++)
	{
		DTNMP_DEBUG_INFO("netui_construct_time_rule_by_idx","Found argument %s",args[x]);
		//Fill into temp command
		char* arrayStart = strchr(args[x],'{');
		if(arrayStart==NULL)
		{
			DTNMP_DEBUG_ERR("netui_construct_time_rule_by_idx","Couldn't find array start",NULL);
			break; //Keep going with what we have
		}
		char* arrayEnd = strchr(arrayStart,'}');
		if(arrayEnd==NULL)
		{
			DTNMP_DEBUG_ERR("netui_construct_time_rule_by_idx","Couldn't find array end %s",arrayStart);
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

		netui_parse_single_mid_str(mids,(char*)&mid_str[0],arguments,lyst_length(gAdmData)-1, MID_TYPE_DATA);


		//mids = netui_parse_mid_str(curCmd,midIdxC, lyst_length(gAdmData))-1, MID_TYPE_DATA)
	}

/*	mids = netui_parse_mid_str(curCmd,midIdxC, lyst_length(gAdmData))-1, MID_TYPE_DATA);

	/* Step 2: Construct the control primitive. */
	rule_time_prod_t *entry = rule_create_time_prod_entry(offset, evals, period, mids);

	/* Step 3: Construct a PDU to hold the primitive. */
	uint8_t *data = ctrl_serialize_time_prod_entry(entry, &size);
	pdu_msg_t *pdu_msg = pdu_create_msg(MSG_TYPE_CTRL_PERIOD_PROD, data, size, NULL);
	pdu_group_t *pdu_group = pdu_create_group(pdu_msg);

	/* Step 4: Send the PDU. */
	iif_send(&ion_ptr, pdu_group, agent->agent_eid.name);

	/* Step 5: Release remaining resources. */
	pdu_release_group(pdu_group);
	rule_release_time_prod_entry(entry);

	SRELEASE(args);

	DTNMP_DEBUG_EXIT("ui_construct_time_rule_by_idx","->.", NULL);
}



///******************************************************************************
// *
// * \par Function Name: ui_construct_time_rule_by_mid
// *
// * \par Constructs a "time production report" control by building a MID. Put
// *      the control in a PDU and sends to agent.
// *
// * \par Notes:
// *	\todo Add ability to apply ACL.
// *
// * Modification History:
// *  MM/DD/YY  AUTHOR         DESCRIPTION
// *  --------  ------------   ---------------------------------------------
// *  01/18/13  E. Birrane     Initial Implementation
// *  06/25/13  E. Birrane     Renamed message "bundle" message "group".
// *****************************************************************************/
//void ui_construct_time_rule_by_mid(agent_t* agent)
//{
//	char line[256];
//	time_t offset = 0;
//	uint32_t period = 0;
//	uint32_t evals = 0;
//	char mid_str[256];
//	Lyst mids = lyst_create();
//	mid_t *midp = NULL;
//	uint32_t size = 0;
//
//	if(agent == NULL)
//	{
//		DTNMP_DEBUG_ENTRY("ui_construct_time_rule_by_mid","(NULL)", NULL);
//		DTNMP_DEBUG_ERR("ui_construct_time_rule_by_mid", "Null EID", NULL);
//		DTNMP_DEBUG_EXIT("ui_construct_time_rule_by_mid","->.", NULL);
//		return;
//	}
//	DTNMP_DEBUG_ENTRY("ui_construct_time_rule_by_mid","(%s)", agent->agent_eid.name);
//
//	/* Step 0: Read the user input. */
//	if(ui_get_user_input("Enter rule as follows: Offset Period #Evals MID",
//			             (char **) &line, 256) == 0)
//	{
//		DTNMP_DEBUG_ERR("ui_construct_time_rule_by_mid","Unable to read user input.", NULL);
//		DTNMP_DEBUG_EXIT("ui_construct_time_rule_by_mid","->.", NULL);
//		return;
//	}
//
//	/* Step 1: Parse the user input. */
//	sscanf(line,"%ld %d %d %s", &offset, &period, &evals, mid_str);
//	midp = ui_build_mid(mid_str);
//
//	char *str = mid_to_string(midp);
//	printf("MID IS %s\n", str);
//	SRELEASE(str);
//
//	lyst_insert_last(mids,midp);
//
//	/* Step 2: Construct the control primitive. */
//	rule_time_prod_t *entry = rule_create_time_prod_entry(offset, evals, period, mids);
//
//	/* Step 3: Construct a PDU to hold the primitive. */
//	uint8_t *data = ctrl_serialize_time_prod_entry(entry, &size);
//	pdu_msg_t *pdu_msg = pdu_create_msg(MSG_TYPE_CTRL_PERIOD_PROD, data, size, NULL);
//	pdu_group_t *pdu_group = pdu_create_group(pdu_msg);
//
//	/* Step 4: Send the PDU. */
//	iif_send(&ion_ptr, pdu_group, agent->agent_eid.name);
//
//	/* Step 5: Release remaining resources. */
//	pdu_release_group(pdu_group);
//	rule_release_time_prod_entry(entry);
//
//	DTNMP_DEBUG_EXIT("ui_construct_time_rule_by_mid","->.", NULL);
//}

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
	char line[MAX_EID_LEN];
	eid_t agent_eid;
	agent_t *agent;

	DTNMP_DEBUG_ENTRY("register_agent", "()", NULL);


	/* Check if the agent is already known. */

	sscanf(curCmd->eid, "%s", agent_eid.name);
	mgr_agent_add(agent_eid);

	DTNMP_DEBUG_EXIT("register_agent", "->.", NULL);
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
	char line[MAX_EID_LEN];

	DTNMP_DEBUG_ENTRY("ui_deregister_agent","(%llu)", (unsigned long)agent);

	if(agent == NULL)
	{
		DTNMP_DEBUG_ERR("ui_deregister_agent", "No agent specified.", NULL);
		DTNMP_DEBUG_EXIT("ui_deregister_agent","->.",NULL);
		return;
	}
	DTNMP_DEBUG_ENTRY("ui_deregister_agent","(%s)",agent->agent_eid.name);

	lockResource(&agents_mutex);

	if(mgr_agent_remove(&(agent->agent_eid)) != 0)
	{
		DTNMP_DEBUG_WARN("ui_deregister_agent","No agent by that name is currently registered.\n", NULL);
	}
	else
	{
		DTNMP_DEBUG_ALWAYS("ui_deregister_agent","Successfully deregistered agent.\n", NULL);
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
void ui_eventLoop()
{
	DTNMP_DEBUG_ERR("uiThread","running",NULL);

//	configure sockets
	int conSock = socket(AF_INET,SOCK_STREAM,0);
	int dataSock = 0;
	short moreData=0;
	socklen_t peerAddrSize = sizeof(struct sockaddr_in);
	struct sockaddr_in localAddr,peerAddr;
	unsigned short port = 12345;
	char cmdBuffer[D_CMDBUFSIZE];
	char* commandReentry=&cmdBuffer[0];

	if(conSock == -1)
	{
		printf("Socket error: Could not create initial listener\n");
		return;
	}
	reUseAddress(conSock);
	memset(&localAddr,0,sizeof(sockaddr_in));

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

	while(g_running)
	{
		//Check for connections, or use valid one
		if(dataSock==0)
		{
			DTNMP_DEBUG_INFO("ui_eventloop","Waiting for connection...",NULL);
			dataSock=accept(conSock,(struct sockaddr*)&peerAddr,&peerAddrSize);
			if(dataSock==-1)
			{
				printf("could not create data socket\n");
				return;
			}
			else
			{
				DTNMP_DEBUG_INFO("ui_eventloop","connected",NULL);
				struct timeval tv={1,0};
				if(setsockopt(dataSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval)) == -1)
				{
					DTNMP_DEBUG_ERR("netui_eventloop","Could not set receive timeout",NULL);
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
			DTNMP_DEBUG_ERR("netui_eventloop","Found variables",NULL);
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
					DTNMP_DEBUG_ERR("netui_eventloop","Invalid data pointer",NULL);
					continue;
				}


				switch(curEntry->type)
				{
					case TYPE_INT32: netui_print_int32(curEntry->value,curEntry->size,varValue);varTextRep="int32";break;
					case TYPE_UINT32: netui_print_uint32(curEntry->value,curEntry->size,varValue);varTextRep="uint32"; break;
					case TYPE_STRING: netui_print_string(curEntry->value,curEntry->size,varValue); varTextRep="string";break;
					case TYPE_DATALIST: netui_print_datalist(curEntry->value,curEntry->size,varValue); varTextRep="tdc";break;

				}
				DTNMP_DEBUG_INFO("netui_eventloop","Variable: %s",curEntry->name);
				uint16_t varStrSize = snprintf(varBuffer,D_VARSTRSIZE,"v:%s@%u\\%s(%s)=%s;\n",curEntry->producer_eid.name,curEntry->timestamp,curEntry->name,varTextRep,varValue);



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
			DTNMP_DEBUG_INFO("netui_eventloop","Grabbing additional data",NULL);
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
					DTNMP_DEBUG_INFO("netui_eventloop","No data, continuing",NULL);
					continue;
				}
				close(dataSock);
				dataSock=0;
				DTNMP_DEBUG_INFO("netui_eventloop","Socket broken, continuing",NULL);
				continue;
			}

			commandReentry=&cmdBuffer[0];
		}

		cmdFormat* curCmd=netui_create_cmdformat();
		moreData = netui_build_command(&commandReentry,curCmd,sizeof(cmdFormat));

		if(moreData == 0)
		{
			netui_free_cmdformat(curCmd);
			DTNMP_DEBUG_WARN("netui_eventLoop","Invalid command",NULL);
			continue;
		}

		short cmdIdx = 0;
		char* curChunk = (curCmd->cmdChunks[cmdIdx]);

		if(&curChunk==NULL)
		{
			netui_free_cmdformat(curCmd);
			continue;
		}
		DTNMP_DEBUG_INFO("ui_eventLoop","command has %d tokens, first is \"%s\" %d",curCmd->numChunks,(char*)(curCmd->cmdChunks[cmdIdx]),&(curCmd->cmdChunks[cmdIdx]));

		/* notes on top level structure:
		<agent>.manager. - For reg/dereg
		<agent>.reports. - For reporting
		<agent>.controls. - For control transmission
		*/
		DTNMP_DEBUG_INFO("netui_eventloop","%d number of chunks",curCmd->numChunks);
		NETUI_SECTION("manager")
		{
			NETUI_DEF_ACTION("register",netui_register_agent(curCmd));
			NETUI_DEF_ACTION("deregister",ui_deregister_agent(netui_find_agent_by_name(curCmd->eid)));
		}

		NETUI_SECTION("controls")
		{
			NETUI_DEF_ACTION("create",netui_construct_ctrl_by_idx(netui_find_agent_by_name(curCmd->eid),curCmd));
		}

		NETUI_SECTION("debug")
		{
			NETUI_DEF_ACTION("controls",ui_print_ctrls());
			NETUI_DEF_ACTION("datarefs",ui_print_mids());
		}

		NETUI_SECTION("reports")
		{
				DTNMP_DEBUG_INFO("netui_eventloop","In reports",NULL);
			NETUI_DEF_ACTION("time",netui_construct_time_rule_by_idx(netui_find_agent_by_name(curCmd->eid),curCmd));
			NETUI_DEF_ACTION("show",netui_print_reports(netui_find_agent_by_name(curCmd->eid)));
			NETUI_DEF_ACTION("number",netui_get_num_reports_by_agent());
			NETUI_DEF_ACTION("delete",ui_clear_reports(netui_find_agent_by_name(curCmd->eid)));
		}

		netui_free_cmdformat(curCmd);

	}
}

//
///******************************************************************************
// *
// * \par Function Name: ui_get_user_input
// *
// * \par Read a line of input from the user.
// *
// * \par Notes:
// *
// * \retval 0 - Could not get user input.
// * 		   1 - Got user input.
// *
// * \param[in]  prompt   The prompt to the user.
// * \param[out] line     The line of text read from the user.
// * \param [in] max_len  The maximum size of the line.
// *
// * Modification History:
// *  MM/DD/YY  AUTHOR         DESCRIPTION
// *  --------  ------------   ---------------------------------------------
// *  01/18/13  E. Birrane     Initial Implementation
// *****************************************************************************/
//
//int ui_get_user_input(char *prompt, char **line, int max_len)
//{
//	int len = 0;
//
//	DTNMP_DEBUG_ENTRY("ui_get_user_input","(%s,0x%x,%d)",prompt, *line, max_len);
//
//	while(len == 0)
//	{
//		printf("Note: Only the first %d characters will be read.\n%s\n",
//				max_len, prompt);
//
//		if (igets(fileno(stdin), (char *)line, max_len, &len) == NULL)
//		{
//			if (len != 0)
//			{
//				DTNMP_DEBUG_ERR("ui_get_user_input","igets failed.", NULL);
//				DTNMP_DEBUG_EXIT("ui_get_user_input","->0.",NULL);
//				return 0;
//			}
//		}
//	}
//
//	DTNMP_DEBUG_INFO("ui_get_user_input","Read user input.", NULL);
//
//	DTNMP_DEBUG_EXIT("ui_get_user_input","->1.",NULL);
//	return 1;
//}

//
//
///******************************************************************************
// *
// * \par Function Name: ui_input_mid
// *
// * \par Construct a MID object completely from user input.
// *
// * \par Notes:
// *
// * \retval NULL  - Problem building the MID.
// * 		   !NULL - The constructed MID.
// *
// * Modification History:
// *  MM/DD/YY  AUTHOR         DESCRIPTION
// *  --------  ------------   ---------------------------------------------
// *  01/18/13  E. Birrane     Initial Implementation
// *  06/25/13  E. Birrane     Removed references to priority field. Add ISS flag.
// *****************************************************************************/
//
//mid_t *ui_input_mid()
//{
//	uint8_t flag;
//	char line[256];
//	uint32_t size = 0;
//	uint8_t *data = NULL;
//	mid_t *result = NULL;
//	uint32_t bytes = 0;
//
//	/* Step 0: Allocate the resultant MID. */
//	if((result = (mid_t*)STAKE(sizeof(mid_t))) == NULL)
//	{
//		DTNMP_DEBUG_ERR("ui_input_mid","Can't alloc %d bytes.", sizeof(mid_t));
//		DTNMP_DEBUG_EXIT("ui_input_mid", "->NULL.", NULL);
//		return NULL;
//	}
//	else
//	{
//		memset(result,0,sizeof(mid_t));
//	}
//
//	/* Step 1: Get the MID flag. */
//	ui_input_mid_flag(&(result->flags));
//	result->type = MID_GET_FLAG_TYPE(result->flags);
//	result->category = MID_GET_FLAG_CAT(result->flags);
//
//	/* Step 2: Grab Issuer, if necessary. */
//	if(MID_GET_FLAG_ISS(result->flags))
//	{
//		ui_get_user_input("Issuer (up to 18 hex): 0x", (char**)&line, 256);
//		data = utils_string_to_hex((unsigned char*)line, &size);
//		memcpy(&(result->issuer), data, 4);
//		SRELEASE(data);
//
//		if(size > 4)
//		{
//			DTNMP_DEBUG_ERR("ui_input_mid", "Issuer too big: %d.", size);
//			DTNMP_DEBUG_EXIT("ui_input_mid","->NULL.", NULL);
//			mid_release(result);
//			return NULL;
//		}
//	}
//
//	/* Step 3: Grab the OID. */
//	ui_get_user_input("OID: 0x", (char**)&line, 256);
//	data = utils_string_to_hex((unsigned char *)line, &size);
//	result->oid = NULL;
//
//	switch(MID_GET_FLAG_OID(result->flags))
//	{
//		case OID_TYPE_FULL:
//			result->oid = oid_deserialize_full(data, size, &bytes);
//			printf("OID value size is %d\n", result->oid->value_size);
//			break;
//		case OID_TYPE_PARAM:
//			result->oid = oid_deserialize_param(data, size, &bytes);
//			break;
//		case OID_TYPE_COMP_FULL:
//			result->oid = oid_deserialize_comp(data, size, &bytes);
//			break;
//		case OID_TYPE_COMP_PARAM:
//			result->oid = oid_deserialize_comp_param(data, size, &bytes);
//			break;
//		default:
//			DTNMP_DEBUG_ERR("ui_input_mid","Unknown OID Type %d",
//						    MID_GET_FLAG_OID(result->flags));
//			break;
//	}
//	SRELEASE(data);
//
//	if((result->oid == NULL) || (bytes != size))
//	{
//		DTNMP_DEBUG_ERR("ui_input_mid", "Bad OID. Size %d. Bytes %d.",
//				        size, bytes);
//		mid_release(result);
//
//		DTNMP_DEBUG_EXIT("ui_input_mid","->NULL.", NULL);
//		return NULL;
//	}
//
//	/* Step 4: Grab a tag, if one exists. */
//	if(MID_GET_FLAG_TAG(result->flags))
//	{
//		ui_get_user_input("Tag (up to 18 hex): 0x", (char**)&line, 256);
//		data = utils_string_to_hex((unsigned char*)line, &size);
//		memcpy(&(result->tag), data, 4);
//		SRELEASE(data);
//
//		if(size > 4)
//		{
//			DTNMP_DEBUG_ERR("ui_input_mid", "Tag too big: %d.", size);
//			DTNMP_DEBUG_EXIT("ui_input_mid","->NULL.", NULL);
//			mid_release(result);
//			return NULL;
//		}
//	}
//
//	mid_internal_serialize(result);
//
//	/* Step 5: Sanity check this mid. */
//	if(mid_sanity_check(result) == 0)
//	{
//		DTNMP_DEBUG_ERR("ui_input_mid", "Sanity check failed.", size);
//		DTNMP_DEBUG_EXIT("ui_input_mid","->NULL.", NULL);
//		mid_release(result);
//		return NULL;
//	}
//
//	char *mid_str = mid_to_string(result);
//	DTNMP_DEBUG_ALWAYS("ui_input_mid", "Defined MID: %s", mid_str);
//	SRELEASE(mid_str);
//
//	DTNMP_DEBUG_EXIT("ui_input_mid", "->0x%x", (unsigned long) result);
//	return result;
//}
//
//
//
///******************************************************************************
// *
// * \par Function Name: ui_input_mid_flag
// *
// * \par Construct a MID flag byte completely from user input.
// *
// * \par Notes:
// *
// * \retval 0 - Can't construct flag byte
// * 		   1 - Flag byte constructed
// *
// * \param[out]  flag   The resulting flags
// *
// * Modification History:
// *  MM/DD/YY  AUTHOR         DESCRIPTION
// *  --------  ------------   ---------------------------------------------
// *  01/18/13  E. Birrane     Initial Implementation
// *  06/25/13  E. Birrane     Removed references to priority field.Add ISS Flag.
// *****************************************************************************/
//
//int ui_input_mid_flag(uint8_t *flag)
//{
//	int result = 0;
//	char line[256];
//	int tmp;
//
//	*flag = 0;
//
//	ui_get_user_input("Type: Data (0), Ctrl (1), Literal (2), Op (3):",
//			          (char**)&line, 256);
//	sscanf(line,"%d",&tmp);
//	*flag = (tmp & 0x3);
//
//	ui_get_user_input("Cat: Atomic (0), Computed (1), 	 (2):",
//			          (char**)&line, 256);
//	sscanf(line,"%d",&tmp);
//	*flag |= (tmp & 0x3) << 2;
//
//	ui_get_user_input("Issuer Field Present? Yes (1)  No (0):",
//			          (char**)&line, 256);
//	sscanf(line,"%d",&tmp);
//	*flag |= (tmp & 0x1) << 4;
//
//	ui_get_user_input("Tag Field Present? Yes (1)  No (0):",
//			          (char**)&line, 256);
//	sscanf(line,"%d",&tmp);
//	*flag |= (tmp & 0x1) << 5;
//
//	ui_get_user_input("OID Type: Full (0), Parm (1), Comp (2), Parm+Comp(3):",
//			          (char**)&line, 256);
//	sscanf(line,"%d",&tmp);
//	*flag |= (tmp & 0x3) << 6;
//
//	printf("Constructed Flag Byte: 0x%x\n", *flag);
//
//	return 1;
//}


/******************************************************************************
 *
 * \par Function Name: ui_print_agents
 *
 * \par Prints list of known agents
 *
 * \par Returns number of agents
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  04/18/13  V.Ramachandran Initial Implementation
 *****************************************************************************/

int ui_print_agents()
{
  int i = 1;
  LystElt element;

  DTNMP_DEBUG_ENTRY("ui_print_agents","()",NULL);

  printf("\n------------- Known Agents --------------\n");

  element = lyst_first(known_agents);
  if(element == NULL)
  {
	  printf("[None]\n");
  }
  while(element != NULL)
  {
	  printf("%d) %s\n", i++, (char *) lyst_data(element));
	  element = lyst_next(element);
  }

  printf("\n------------- ************ --------------\n");
  printf("\n");

  DTNMP_DEBUG_EXIT("ui_print_agents","->%d", (i-1));
  return i;
}

/******************************************************************************
 *
 * \par Function Name: ui_print_ctrls
 *
 * \par Prints list of configured controls and their associated index
 *
 * \par Notes:
 * 	1. Assuming 80 column display, 2 column are printed of length 40 each.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/

void ui_print_ctrls()
{
  int i = 0;
  int num_full_rows = 0;
  int num_rows = 0;
  LystElt elt = 0;
  adm_ctrl_t *cur = NULL;

  DTNMP_DEBUG_ENTRY("ui_print_ctrls","()",NULL);

  num_full_rows = (int) (lyst_length(gAdmCtrls) / 2);

  for(elt = lyst_first(gAdmCtrls); elt; elt = lyst_next(elt))
  {
	  cur = (adm_ctrl_t*) lyst_data(elt);
	  printf("%3d) %-35s ", i, cur->name);
	  i++;

	  if(num_rows < num_full_rows)
	  {
		  elt = lyst_next(elt);
		  cur = (adm_ctrl_t*) lyst_data(elt);
		  printf("%3d) %-35s\n", i, cur->name);
		  i++;
	  }
	  else
	  {
		  printf("\n\n\n");
	  }
	  num_rows++;
  }

  DTNMP_DEBUG_EXIT("ui_print_ctrls","->.", NULL);
}



/******************************************************************************
 *
 * \par Function Name: ui_print_custom_rpt
 *
 * \par Prints a custom report received by a DTNMP Agent.
 *
 * \par Notes:
 *
 * \param[in]  rpt_entry  The entry containing the report data to print.
 * \param[in]  rpt_def    The static definition of the report.
 *
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/

void netui_print_custom_rpt(rpt_data_entry_t *rpt_entry, def_gen_t *rpt_def)
{
	LystElt elt;
	uint64_t idx = 0;
	mid_t *cur_mid = NULL;
	adm_datadef_t *adu = NULL;
	uint64_t data_used;

	for(elt = lyst_first(rpt_def->contents); elt; elt = lyst_next(elt))
	{
		char *mid_str;
		cur_mid = (mid_t*)lyst_data(elt);
		mid_str = mid_to_string(cur_mid);
		if((adu = adm_find_datadef(cur_mid)) != NULL)
		{
			DTNMP_DEBUG_INFO("ui_print_custom_rpt","Printing MID %s", mid_str);
		//	netui_print_predefined_rpt(cur_mid, (uint8_t*)&(rpt_entry->contents[idx]),
		//			             rpt_entry->size - idx, &data_used, adu);
			idx += data_used;
		}
		else
		{
			DTNMP_DEBUG_ERR("ui_print_custom_rpt","Unable to find MID %s", mid_str);
		}

		SRELEASE(mid_str);
	}
}


/******************************************************************************
 *
 * \par Function Name: ui_print_mids
 *
 * \par Prints list of configured data items and their associated index
 *
 * \par Notes:
 * 	1. Assuming 80 column display, 2 column are printed of length 40 each.
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/

void ui_print_mids()
{
	int i = 0;
	int num_full_rows = 0;
	int num_rows = 0;
	LystElt elt = 0;
	adm_datadef_t *cur = NULL;

	DTNMP_DEBUG_ENTRY("ui_print_mids","()",NULL);


	num_full_rows = (int) (lyst_length(gAdmData) / 2);

	for(elt = lyst_first(gAdmData); elt; elt = lyst_next(elt))
	{
		cur = (adm_datadef_t*) lyst_data(elt);
		printf("%3d) %-35s ", i, cur->name);
		i++;

		if(num_rows < num_full_rows)
		{
			elt = lyst_next(elt);
			cur = (adm_datadef_t*) lyst_data(elt);
			printf("%3d) %-35s\n", i, cur->name);
			i++;
		}
		else
		{
			printf("\n\n\n");
		}
		num_rows++;
	}

	DTNMP_DEBUG_EXIT("ui_print_mids","->.", NULL);
}



/******************************************************************************
 *
 * \par Function Name: ui_print_predefined_rpt
 *
 * \par Prints a pre-defined report received by a DTNMP Agent.
 *
 * \par Notes:
 *
 * \param[in]  mid        The identifier of the data item being printed.
 * \param[in]  data       The contents of the data item.
 * \param[in]  data_size  The size of the data to be printed.
 * \param[out] data_used  The bytes of the data consumed by printing.
 * \param[in]  adu        The static definition of the report.
 *
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  01/18/13  E. Birrane     Initial Implementation
 *****************************************************************************/

void netui_print_predefined_rpt(mid_t *mid, uint8_t *data, uint64_t data_size, uint64_t *data_used, adm_datadef_t *adu,eid_t* eid,time_t time)
{
	uint64_t len;
	char* mid_str = NULL;
	variableQueueEntry  *mid_val = NULL;
	uint32_t val_size = adu->get_size(data, data_size);
	uint32_t str_size = 0;

	if((mid_val = adu->to_string(data, data_size, val_size, &str_size,adu->name)) == NULL)
	{
		DTNMP_DEBUG_ERR("ui_print_predefined_rpt","Can't copy varentry",NULL);

		SRELEASE(mid_str);
		return;
	}
	else
	{

		if(eid!=NULL)
		{
			DTNMP_DEBUG_INFO("report_print","Copying eid",NULL);
			memcpy(&mid_val->producer_eid,eid,sizeof(eid_t));
			DTNMP_DEBUG_INFO("report_print","eid successful",NULL);
		}

		mid_val->timestamp=time;
		//DTNMP_DEBUG_INFO("value","%d name: %s",mid_val->value,*adu->name)
		DTNMP_DEBUG_INFO("report_print","Adding to queue",NULL);
		AddVariableToQueue(mid_val);
		DTNMP_DEBUG_INFO("report_print","successful",NULL);
	}

}



/******************************************************************************
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
	 rpt_data_t *cur_report = NULL;
	 rpt_data_entry_t *cur_entry = NULL;

	 if(agent == NULL)
	 {
		 DTNMP_DEBUG_ENTRY("ui_print_reports","(NULL)", NULL);
		 DTNMP_DEBUG_ERR("ui_print_reports", "No agent specified", NULL);
		 DTNMP_DEBUG_EXIT("ui_print_reports", "->.", NULL);
		 return;

	 }
	 DTNMP_DEBUG_ENTRY("ui_print_reports","(%s)", agent->agent_eid.name);

	 if(lyst_length(agent->reports) == 0)
	 {
		 DTNMP_DEBUG_ALWAYS("ui_print_reports","[No reports received from this agent.]", NULL);
		 DTNMP_DEBUG_EXIT("ui_print_reports", "->.", NULL);
		 return;
	 }

	 /* Free any reports left in the reports list. */
	 for (report_elt = lyst_first(agent->reports); report_elt; report_elt = lyst_next(report_elt))
	 {
		 /* Grab the current report */
	     if((cur_report = (rpt_data_t*)lyst_data(report_elt)) == NULL)
	     {
	        DTNMP_DEBUG_ERR("ui_print_reports","Unable to get report from lyst!", NULL);
	     }
	     else
	     {
	    	 unsigned long mid_sizes = 0;
	    	 unsigned long data_sizes = 0;
	    	 adm_datadef_t *adu = NULL;
	    	 def_gen_t *report = NULL;


			/* Print the Report Header */
	    	//AddVariableToQueue("SentTo",TYPE_STRING,cur_report->recipient.name,&agent->agent_eid,strlen(cur_report->recipient.name),cur_report->time);
	    	//AddVariableToQueue("NumMids",TYPE_UINT32,lyst_length(cur_report->reports),agent->agent_eid.name);
	    	/*
	    	 printf("\n-----------------\nDTNMP DATA REPORT\n-----------------\n");
	    	 printf("Sent to  : %s\n", cur_report->recipient.name);
	    	 printf("Rpt. Size: %d\n", cur_report->size);
	    	 printf("Timestamp: %ld\n", cur_report->time);
	    	 printf("Num Mids : %ld\n", lyst_length(cur_report->reports));
	    	 printf("Value(s)\n---------------------------------\n");
*/

	    	 /* For each MID in this report, print and deleteit. */
	    	 for(entry_elt = lyst_first(cur_report->reports); entry_elt; entry_elt = lyst_next(entry_elt))
	    	 {
	    		 cur_entry = (rpt_data_entry_t*)lyst_data(entry_elt);

	    		 mid_sizes += cur_entry->id->raw_size;
	    		 data_sizes += cur_entry->size;

	    		 /* See if this is a pre-defined report, or a custom report. */
	    		 /* Find ADM associated with this entry. */
	    		 if((adu = adm_find_datadef(cur_entry->id)) != NULL)
	    		 {
	    			 uint64_t used;
	    			 netui_print_predefined_rpt(cur_entry->id, cur_entry->contents, cur_entry->size, &used, adu,&agent->agent_eid,cur_report->time);

	    		 }
	    		 else if((report = def_find_by_id(agent->custom_defs, &(agent->mutex), cur_entry->id)) != NULL)
	    		 {
	    			 netui_print_custom_rpt(cur_entry, report);
	    		 }
	    		 else
	    		 {
	    			 char *mid_str = mid_to_string(cur_entry->id);
	    			 DTNMP_DEBUG_ERR("ui_print_reports","Could not print MID %s", mid_str);
	    			 SRELEASE(mid_str);
	    		 }
	    	 }
	    	 printf("=================\n");
	    	 printf("STATISTICS:\n");
	    	 printf("MIDs total %ld bytes\n", mid_sizes);
	    	 printf("Data total: %ld bytes\n", data_sizes);
	    	 printf("Efficiency: %.2f%%\n", (double)(((double)data_sizes)/((double)cur_report->size)) * (double)100.0);
	    	 printf("-----------------\n\n\n");

	    	 //previousReportElt=lyst_prev(report_elt);
	    	 //lyst_delete(report_elt);
	    	 //report_elt=previousReportElt;
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

	oid_t *oid = oid_deserialize_full(tmp_oid, 8, &bytes);

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

	mid_t *mid = mid_construct(0,0, NULL, NULL, oid);
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



void *net_ui_thread(void * threadId)
{
	DTNMP_DEBUG_ENTRY("ui_thread","(0x%x)", (unsigned long) threadId);

	ui_eventLoop();

	DTNMP_DEBUG_EXIT("ui_thread","->.", NULL);
	pthread_exit(NULL);
}
/******************************************************************************
 *
 * \par Function Name: netui_build_command
 *
 * \par Purpose: The core of the lazyRPC system, this function takea a buffer, and creates a cmdFormat.
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
		DTNMP_DEBUG_ERR("Netui_build_command","Invalid command",NULL);
		return 0;
	}
	//DTNMP_DEBUG_INFO("build","%s",buffer);
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

	//DTNMP_DEBUG_INFO("netui_build_command, found EID","%s",cmdOutput->eid)
	if(cmdOutput->eid==NULL)
	{
		//SRELEASE(inputBuffer);
		return 0;
	}
	inputBuffer=strtok(NULL,"\\=");

	for(;;inputBuffer=NULL)
	{
		DTNMP_DEBUG_ERR("netui_build_command","loop",NULL);
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
		DTNMP_DEBUG_ERR("netui_build_command","loop3",NULL);
	//Now, do the parsing

		tokSize = strlen(curTok);
		if(tokSize==0)
		{
			DTNMP_DEBUG_ERR("netui_build_command","Undefined string, continuing",NULL);
			continue;
		}

		cmdOutput->cmdChunks[cmdIdx] = (char*)STAKE(tokSize);
		sscanf(curTok,"%s",cmdOutput->cmdChunks[cmdIdx]);
		DTNMP_DEBUG_INFO("Netui_build_command","Copied token (%d): %s",cmdIdx,cmdOutput->cmdChunks[cmdIdx]);
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
	DTNMP_DEBUG_ENTRY("netui_free_cmdformat","Freeing %#u",toFree);
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

	DTNMP_DEBUG_ENTRY("netui_find_agent_by_name",NULL,NULL);

	element = lyst_first(known_agents);
	if(element == NULL)
	{
	  return NULL;
	}
	while(element != NULL)
	{
	  if(strcmp((char *) lyst_data(element),name)==0)
	  {
	  	DTNMP_DEBUG_INFO("netui_find_agent_by_name found element",NULL,NULL);
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
	adm_ctrl_t *cur = NULL;

	DTNMP_DEBUG_ENTRY("netui_find_ctrl_idx_by_name","(%s)",name);

	DTNMP_DEBUG_INFO("netui_find_ctrl_idx_by_name","%s.", name);

	for(elt = lyst_first(gAdmCtrls); elt; elt = lyst_next(elt))
	{
		cur = (adm_ctrl_t*) lyst_data(elt);
		if(strcasecmp(cur->name,name)==0)
			return i; //Found it!
	  i++;

	}
	DTNMP_DEBUG_INFO("netui_find_ctrl_idx_by_name","finding %s failed.", name);
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
	adm_ctrl_t *cur = NULL;

	DTNMP_DEBUG_ENTRY("netui_find_data_idx_by_name","(%s)",name);

	DTNMP_DEBUG_INFO("netui_find_data_idx_by_name","%s.", name);

	for(elt = lyst_first(gAdmData); elt; elt = lyst_next(elt))
	{
		cur = (adm_ctrl_t*) lyst_data(elt);
		if(strcasecmp(cur->name,name)==0)
			return i; //Found it!
	  i++;

	}
	DTNMP_DEBUG_INFO("netui_find_data_idx_by_name","finding %s failed.", name);
	return -1;
}

/******************************************************************************
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

	DTNMP_DEBUG_ENTRY("netui_define_raw_mid_params", "(0x%x, %d, 0x%x)",
			          (unsigned long) name, num_parms, (unsigned long) mid);

	if((name == NULL) || (mid == NULL))
	{
		DTNMP_DEBUG_ERR("netui_define_mid_params", "Bad Args.", NULL);
		DTNMP_DEBUG_EXIT("netui_define_mid_params","->.", NULL);
		return;
	}

//	Parse arguments
	allArgs = netui_parse_arguments(arguments,&numArgs);
	if(numArgs<num_parms)
	{
		DTNMP_DEBUG_WARN("Not enough parameters","",NULL);
		free(allArgs);
		return;
	}
	for(i = 0; i < num_parms; i++)
	{

		//If the string is opened with a {, then its an array... We don't really care (we pass it as a single string), except in the cases where it is an array of datalists
		if(strspn(allArgs[i],"{[")==2)
		{
			DTNMP_DEBUG_INFO("netui_define_raw_mid_params","In datalist processor",NULL);
			//char* arrayReentry;
			Lyst dlDatacol = lyst_create();
			datacol_entry_t* datacolEntry;
			uint8_t numDatalists;
			char* arrayStart = strchr(allArgs[i],'{');

			if(arrayStart==NULL)
			{
				DTNMP_DEBUG_ERR("netui_define_raw_mid_params","Couldn't find array start",NULL);
				lyst_destroy(dlDatacol);
				break; //Keep going with what we have
			}
			arrayStart++;
			char* arrayEnd = strchr(arrayStart,'}');
			if(arrayEnd==NULL)
			{
				DTNMP_DEBUG_ERR("netui_define_raw_mid_params","Couldn't find array end %s",arrayStart);
				lyst_destroy(dlDatacol);
				break; //Keep going with what we have
			}
			//Delimit array
			arrayEnd[0]='\0';

			char** datalists = netui_parse_arguments(arrayStart,&numDatalists);

			DTNMP_DEBUG_INFO("netui_define_mid_params","Found # datalists: %d in %s",numDatalists,arrayStart);
			for(unsigned int x = 0; x < numDatalists ; x++)
			{
				char* arrayTok = datalists[x]+1;

				char* arrayEndBracket = strchr(arrayTok,']');
				if(arrayEndBracket)
					arrayEndBracket[0]='\0';
				else
				{
					DTNMP_DEBUG_INFO("netui_define_mid_params","Failed to parse, continuing",NULL);

					continue;
				}
				DTNMP_DEBUG_INFO("netui_define_mid_params","Found datalist: %s",arrayTok);

				datalist_t curDl = netui_parse_datalist(arrayTok);
				//datacolE7ntry = (datacol_entry_t*)STAKE(sizeof(datacol_entry_t));

				DTNMP_DEBUG_INFO("netui_define_mid_params","Serializing %s",arrayTok);

				datacolEntry = datalist_serialize_to_datacol(&curDl);
				lyst_insert_last(dlDatacol,datacolEntry);
				datalist_free_contents(&curDl);
				DTNMP_DEBUG_INFO("netui_define_mid_params","serializing done",NULL);
			}
			//Now, serialize the datacol
			DTNMP_DEBUG_INFO("netui_define_mid_params","Performing final serialization %d",size);
			paramBuffer=(char*)utils_datacol_serialize(dlDatacol,&size);
			DTNMP_DEBUG_INFO("netui_define_mid_params","Serialized %d bytes to datacol",size);
			utils_datacol_destroy(&dlDatacol);
		}
		else
		{
			paramBuffer=allArgs[i];
			size=strlen(allArgs[i]);
		}
		DTNMP_DEBUG_INFO("netui_define_mid_params","copying arg: %d of size %d",i,size);
    	mid_add_param(mid, (unsigned char*)paramBuffer, size);
	}

	DTNMP_DEBUG_EXIT("ui_define_mid_params","->.", NULL);
}

inline void netui_define_mid_params(cmdFormat* curCmd,char *name, int num_parms, mid_t *mid)
{
	DTNMP_DEBUG_ENTRY("netui_define_mid_params", "(0x%x, %d, 0x%x)",
			          (unsigned long) name, num_parms, (unsigned long) mid);

	netui_define_raw_mid_params(name,curCmd->arguments,num_parms,mid);
	DTNMP_DEBUG_EXIT("ui_define_mid_params","->.", NULL);
}
/******************************************************************************
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

	DTNMP_DEBUG_ENTRY("ui_parse_single_mid_str","(0x%x) %s",mid_str,mid_str);

	/* Step 0: Sanity Check. */
	if(mid_str == NULL)
	{
		DTNMP_DEBUG_ERR("ui_parse_single_mid_str","Bad args.", NULL);
		DTNMP_DEBUG_EXIT("ui_parse_single_mid_str","->NULL.", NULL);
		return;
	}


	DTNMP_DEBUG_INFO("ui_parse_single_mid_str","Read MID index of %s", mid_str);

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
		case MID_TYPE_DATA:
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
		case MID_TYPE_CONTROL:
			{
				adm_ctrl_t *cur = adm_find_ctrl_by_idx(cur_mid_idx);
				midp = mid_copy(cur->mid);
				num_parms = cur->num_parms;

				DTNMP_DEBUG_INFO("netui_parse_single_mid_str","Entered MID_TYPE_CONTROL",NULL);

			/*	name = ctrls[cur_mid_idx].name;
				num_parms = ctrls[cur_mid_idx].num_parms;
				mid_len = ctrls[cur_mid_idx].mid_len;
				midp = mid_deserialize((unsigned char*)&(ctrls[cur_mid_idx].mid),
								   ctrls[cur_mid_idx].mid_len,&bytes);
			*/
			//	midp = mid_deserialize((unsigned char*)midp->raw,mid_len,&bytes);
			}
			break;
		case MID_TYPE_LITERAL:
			/* \todo: Write this. */
		case MID_TYPE_OPERATOR:
			/* \todo: Write this. */
		default:
			DTNMP_DEBUG_ERR("ui_parse_mid_str","Unknown type %d", type);
			DTNMP_DEBUG_EXIT("ui_parse_mid_str","->NULL.",NULL);
			lyst_destroy(mids);
			return;
		}

		/* If this MID has parameters, get them */
		if(num_parms > 0)
		{
			netui_define_raw_mid_params(mid_str,arguments, num_parms, midp);
			mid_internal_serialize(midp);
		}

		mid_size += mid_len;
		lyst_insert_last(mids, midp);
	}
	else
	{
		DTNMP_DEBUG_ERR("ui_parse_mid_str",
						"Bad MID index: %d. Max is %d. Skipping.",
						cur_mid_idx, max_idx);
	}


	DTNMP_DEBUG_EXIT("ui_parse_mid_str","->0x%x.", mids);
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
	DTNMP_DEBUG_INFO("netui_parse_arguments","Working with argument: %s",argString);
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
		DTNMP_DEBUG_INFO("blah","copying arg: %s of size %d",curArg,cursor-argBuf);
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
	DTNMP_DEBUG_INFO("netui_get_num_reports_by_agent","Starting",NULL);

	LystElt elt;
	uint32_t tempValue;
	lockResource(&agents_mutex);

	for(elt = lyst_first(known_agents);elt;elt=lyst_next(elt))
	{
		DTNMP_DEBUG_INFO("netui_get_num_reports_by_agent","in",NULL);
		agent_t* agent = (agent_t*)lyst_data(elt);

		lockResource(&agent->mutex);
		tempValue = lyst_length(agent->reports);
		unlockResource(&agent->mutex);

		AddVariableToQueue("num_reports",TYPE_UINT32,&tempValue,&agent->agent_eid);

	}

	unlockResource(&agents_mutex);
	DTNMP_DEBUG_EXIT("netui_get_num_reports_by_agent","ending",NULL);
}
/******************************************************************************
 *
 * \par Function Name: netui_parse_datalist
 *
 * \par Purpose:given a string containing a datalist, create a datalist (with
 * 				intact typing)
 * \return the datalist
 *
 *
 * \param[in]   dlText		The null-terminated string containing the datalist
 * \par Notes: This exists for the RPC system, and will attempt to fail
 *				gracefully. The user must verify the length of the returned
 *				datalist, or hope that the client validates.
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
datalist_t netui_parse_datalist(char* dlText)
{
	DTNMP_DEBUG_INFO("netui_parse_datalist","Parsing %s",dlText);
	datalist_t outDl=datalist_create(NULL);
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
		datalist_type_t type=datalist_get_type_from_string(typeTxt);


		if(type==DLIST_TYPE_STRING)
		{
			valueSize=strlen(valueTxt);
			//We need to seek to the end of the entry, if it's quoted

		}
		else
			valueSize=datalist_get_size_for_type(type);

		if(valueSize==0)
		{
			DTNMP_DEBUG_ERR("netui_parse_arguments","Couldn't determine type, continuing...",NULL);
			continue;

		}
		//Create a temporary buffer
		char* value = (char*)STAKE(valueSize);
		memset(value,0,valueSize);
		DTNMP_DEBUG_INFO("netui_parse_datalist","Using specifier: \"%s\" for data \"%s\"",datalist_printf_spec_from_type(type),valueTxt);
		sscanf(valueTxt,datalist_printf_spec_from_type(type),value);

		if(datalist_insert_with_type(&outDl,type,value,valueSize)==DLIST_INVALID)
		{
			DTNMP_DEBUG_ERR("netui_parse_argument","Failed to insert, exiting",NULL);
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

size_t netui_print_datalist(void* inBuffer,size_t size,char* outBuffer)
{
	uint32_t bytesUsed;
	uint32_t bytesUsedPerDL;
	Lyst datalists = utils_datacol_deserialize((uint8_t*)inBuffer,size,&bytesUsed);
	uint8_t* data;
	size_t dlEltSize;
	char* cursor = &outBuffer[0];
	unsigned int numBytesPrinted=0;
	//Start the datalist textual form
	(cursor++)[0]='{';
	//This line also adds the appending comma between datalists.
	for(LystElt dlElt = lyst_first(datalists) ; dlElt ; dlElt = lyst_next(dlElt),(cursor++)[0]=',')
	{
		DTNMP_DEBUG_INFO("netui_print_datalist","Getting datalist...",NULL);
		datacol_entry_t* dataCol = (datacol_entry_t*)lyst_data(dlElt);
		datalist_t curDl = datalist_deserialize_from_buffer(dataCol->value,dataCol->length,&bytesUsedPerDL);

		//Alright, now we have a single DL
		(cursor++)[0]='[';
		unsigned int numElements = datalist_num_elements(&curDl)-1;
		for(unsigned int x = 0 ; x<numElements ; x++,(cursor++)[0]=',')
		{
			datalist_type_t type = datalist_get_type(&curDl,x);
			dlEltSize=datalist_get_size(&curDl,x);
			if(dlEltSize==0)
				break;

			data=(uint8_t*)STAKE(dlEltSize);
			datalist_get(&curDl,x,data,&dlEltSize,type);


			cursor+= sprintf(cursor,"(%s) ",datalist_get_string_from_type(type));
			//Call print function
			switch(type)
			{
				case DLIST_TYPE_STRING:
					cursor+= netui_print_string(data,size,cursor);
				break;
				case DLIST_TYPE_UINT32:
					cursor+= netui_print_uint32(data,size,cursor);
				break;
				case DLIST_TYPE_UINT64:
					cursor+= netui_print_uint64(data,size,cursor);
				break;
				case DLIST_TYPE_INT32:
					cursor+= netui_print_int32(data,size,cursor);
				break;
				case DLIST_TYPE_INT64:
					cursor+= netui_print_int64(data,size,cursor);
				break;
				case DLIST_TYPE_REAL32:
					cursor+= netui_print_real32(data,size,cursor);
				break;
				case DLIST_TYPE_REAL64:
					cursor+= netui_print_real64(data,size,cursor);
				break;
				case DLIST_TYPE_VAST:
					cursor+= netui_print_vast(data,size,cursor);
				break;
				case DLIST_TYPE_UVAST:
					cursor+= netui_print_uvast(data,size,cursor);
				break;
			}

			//if(x<numElements-1)
			//	(cursor++)[0]=',';

			SRELEASE(data);
		}
		(cursor++)[0]=']';
	}
	(cursor++)[0]='}';
	(cursor++)[0]='\0';

	return cursor-outBuffer;
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
	return sprintf(outBuffer,"%Lf",*(double*)inBuffer);
}
size_t netui_print_vast(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,VAST_FIELDSPEC,*(vast*)inBuffer);
}
size_t netui_print_uvast(void* inBuffer,size_t size,char* outBuffer)
{
	return sprintf(outBuffer,UVAST_FIELDSPEC,*(uvast*)inBuffer);
}
