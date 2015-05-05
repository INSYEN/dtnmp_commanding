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
 ** \file nm_mgr_netui.h
 **
 **
 ** Description: A text-based DTNMP Manager. This manager provides the
 **              following functions associated with DTN network management:
 **
 **              1. Define and send custom report and macro definitions
 **              2. Provide tools to build all versions of MIDs and OIDs.
 **              3. Configure Agents for time- and state- based production
 **              4. Print data reports received from a DTNMP Agent
 **
 ** Notes:
 **		1. Currently we do not support ACLs.
 **		2. Currently, we do not support multiple DTNMP agents.
 **
 ** Assumptions:
 **
 ** Modification History:
 **  MM/DD/YY  AUTHOR         DESCRIPTION
 **  --------  ------------   ---------------------------------------------
 **  01/18/13  E. Birrane     Code comments and cleanup
 *****************************************************************************/
#include "nm_mgr.h"

#include "shared/utils/nm_types.h"
#include "shared/adm/adm.h"
#include "shared/primitives/mid.h"
#include "shared/primitives/datalist.h"

#define D_CMDBUFSIZE 1024
#define D_VARENTRYSIZE 1024
#define D_VARSTRSIZE 1316
#define NETUI_START_SECTION(name); if(strcasecmp(curChunk,name) == 0) {curChunk=curCmd->cmdChunks[++cmdIdx];
#define NETUI_DEF_ACTION(name,func) {if(strcasecmp(curChunk,name)==0) {func;}};
#define NETUI_END_SECTION() }

#define D_INPUTMAXCHUNKS 128
#define D_INPUTBUFFERSIZE 1024
extern int gContext;

typedef enum
{
	CMD_GET,
	CMD_SET,
	CMD_RESPONSE,
	CMD_END,
} cmdType_T;

typedef struct
{
	char** cmdChunks;
	char* arguments;
	char* eid;
	cmdType_T cmdType;
	unsigned short numChunks;

} cmdFormat;


void ui_clear_reports(agent_t* agent);

agent_t *ui_select_agent();

void netui_construct_ctrl_by_idx(agent_t* agent,cmdFormat* curCmd);
void ui_construct_time_rule_by_idx(agent_t* agent);

void ui_define_macro(agent_t* agent);
void ui_define_report(agent_t* agent);
void ui_define_mid_params(char *name, int num_parms, mid_t *mid);

void netui_register_agent(cmdFormat* curCmd);
void ui_deregister_agent();


void ui_event_loop();

mid_t *ui_input_mid();
int ui_input_mid_flag(uint8_t *flag);

Lyst ui_parse_mid_str(char *mid_str, int max_idx, int type);

void ui_print_ctrls();
int ui_print_agents();
void netui_print_custom_rpt(rpt_data_entry_t *rpt_entry, def_gen_t *rpt_def);
void ui_print_mids();
void netui_print_predefined_rpt(mid_t *mid, uint8_t *data, uint64_t data_size, uint64_t *data_used, adm_datadef_t *adu,eid_t* eid,time_t time);
void netui_print_reports(agent_t *agent);

void ui_run_tests();


void *net_ui_thread(void * threadId);


short netui_build_command(char** inBuffer,cmdFormat* cmdOutput,size_t bufSize);
void netui_free_cmdformat(cmdFormat* toFree);

agent_t* netui_find_agent_by_name(char* name);
int netui_find_ctrl_idx_by_name(char* name);
inline void netui_define_mid_params(cmdFormat* curCmd,char *name, int num_parms, mid_t *mid);
Lyst netui_parse_mid_str(cmdFormat* curCmd,char *mid_str, int max_idx, int type);
int netui_find_data_idx_by_name(char* name);
void netui_construct_time_rule_by_idx(agent_t* agent,cmdFormat* curCmd);
char** netui_parse_arguments(char* argString,uint8_t* numArgsOut);
void netui_parse_single_mid_str(Lyst mids,char *mid_str, char* arguments,int max_idx, int type);
void netui_define_raw_mid_params(char *name, char* arguments,int num_parms, mid_t *mid);
void netui_get_num_reports_by_agent();
datalist_t netui_parse_datalist(char* dlText);
cmdFormat* netui_create_cmdformat();

//Variable queue/datalist printing functions
size_t netui_print_datalist(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_string(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_uint32(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_int32(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_uint64(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_int64(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_real32(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_real64(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_vast(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_uvast(void* inBuffer,size_t size,char* outBuffer);
