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

#include "../shared/utils/nm_types.h"
#include "../shared/adm/adm.h"
#include "../shared/primitives/mid.h"
#include "../shared/primitives/report.h"

#define D_CMDBUFSIZE 2048
#define D_VARENTRYSIZE 2048
#define D_VARSTRSIZE 1316
#define NETUI_DEF_ACTION(name,func) {if(strcasecmp(curCmd->cmdChunks[cmdIdx],name)==0) {func;}};
#define NETUI_SECTION(name) if((strncasecmp(curCmd->cmdChunks[0],name,strlen(name)) == 0) ? ++cmdIdx:0)

#define D_INPUTMAXCHUNKS 128
#define D_INPUTBUFFERSIZE 4096

#define MAX_PARMS 5
#define MAX_PARM_NAME 16

#define UI_ADD_PARMSPEC_1(str, n1, p1) \
	   ui_add_parmspec(str, 1, n1, p1, NULL, 0, NULL, 0, NULL, 0, NULL, 0);

#define UI_ADD_PARMSPEC_2(str, n1, p1, n2, p2) \
	   ui_add_parmspec(str, 2, n1, p1, n2, p2, NULL, 0, NULL, 0, NULL, 0);

#define UI_ADD_PARMSPEC_3(str, n1, p1, n2, p2, n3, p3) \
	   ui_add_parmspec(str, 3, n1, p1, n2, p2, n3, p3, NULL, 0, NULL, 0);

#define UI_ADD_PARMSPEC_4(str, n1, p1, n2, p2, n3, p3, n4, p4) \
	   ui_add_parmspec(str, 4, n1, p1, n2, p2, n3, p3, n4, p4, NULL, 0);

#define UI_ADD_PARMSPEC_5(str, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5) \
	   ui_add_parmspec(str, 5, n1, p1, n2, p2, n3, p3, n4, p4, n5, p5);

extern Lyst gParmSpec;
extern volatile uint8_t gRunning;

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

/*
 * The parameter spec keeps a list of known parameters
 * for individual, known parameterized MIDs.
 *
 * Currently, only controls and literals can be parameterized.
 */
typedef struct
{
	mid_t *mid;
	uint8_t num_parms;
	uint8_t parm_type[MAX_PARMS];
	char parm_name[MAX_PARMS][MAX_PARM_NAME];
} ui_parm_spec_t;


void  ui_add_parmspec(char *mid_str,
						       uint8_t num,
		                       char *n1, uint8_t p1,
		                       char *n2, uint8_t p2,
		                       char *n3, uint8_t p3,
		                       char *n4, uint8_t p4,
		                       char *n5, uint8_t p5);

ui_parm_spec_t* ui_get_parmspec(mid_t *mid);


void ui_clear_reports(agent_t* agent);

agent_t *ui_select_agent();

void netui_construct_ctrl_by_idx(agent_t* agent,cmdFormat* curCmd);
void ui_construct_time_rule_by_idx(agent_t* agent);

void ui_define_macro(agent_t* agent);
void ui_define_report(agent_t* agent);
void ui_define_mid_params(char *name, int num_parms, mid_t *mid);

void netui_register_agent(cmdFormat* curCmd);
void ui_deregister_agent();


void ui_event_loop(int *running);

//mid_t *ui_input_mid();
//int ui_input_mid_flag(uint8_t *flag);

Lyst ui_parse_mid_str(char *mid_str, int max_idx, int type);

void ui_list_adms();
void ui_list_atomic();
void ui_list_compdef();
void ui_list_ctrls();
void ui_list_gen(int adm_type, int mid_id);
void ui_list_literals();
void ui_list_macros();
void ui_list_mids();
void ui_list_ops();
void ui_list_rpts();

void netui_print_entry(rpt_entry_t *entry, eid_t *receiver, time_t ts, uvast *mid_sizes, uvast *data_sizes);
void netui_print_reports(agent_t *agent);

void ui_run_tests();


void *netui_thread(int *running);


short netui_build_command(char** inBuffer,cmdFormat* cmdOutput,size_t bufSize);
cmdFormat* netui_create_cmdformat();
void netui_free_cmdformat(cmdFormat* toFree);

agent_t* netui_find_agent_by_name(char* name);
int netui_find_ctrl_idx_by_name(char* name);
void netui_define_mid_params(cmdFormat* curCmd,char *name, int num_parms, mid_t *mid);
Lyst netui_parse_mid_str(cmdFormat* curCmd,char *mid_str, int max_idx, int type);
int netui_find_data_idx_by_name(char* name);
void netui_construct_time_rule_by_idx(agent_t* agent,cmdFormat* curCmd);
char** netui_parse_arguments(char* argString,uint8_t* numArgsOut);
void netui_parse_single_mid_str(Lyst mids,char *mid_str, char* arguments,int max_idx, int type);
void netui_define_raw_mid_params(char *name, char* arguments,int num_parms, mid_t *mid);
void netui_get_num_reports_by_agent();


tdc_t* netui_parse_tdc(char* dlText);

size_t netui_print_tdc_def(tdc_t* tdc, def_gen_t* cur_def, char* outBuffer);

//Variable queue/tdc printing functions
size_t netui_print_tdc(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_string(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_uint32(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_int32(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_uint64(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_int64(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_real32(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_real64(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_vast(void* inBuffer,size_t size,char* outBuffer);
size_t netui_print_uvast(void* inBuffer,size_t size,char* outBuffer);
