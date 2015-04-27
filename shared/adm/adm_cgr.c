#include "ion.h"
#include "lyst.h"
#include "platform.h"

#include "shared/adm/adm_cgr.h"
#include "shared/utils/utils.h"


void adm_cgr_init()
{
	/* Node-specific Information. */
	uint8_t mid_str[ADM_MID_ALLOC];


//	adm_build_mid_str(0, BP_ADM_DATA_NN, BP_ADM_DATA_NN_LEN, 0, mid_str);
//	adm_add_datadef("BP_NODE_ALL",                 mid_str, 0,  bp_print_node_all,       bp_size_node_all);

	/*Data references*/
	adm_build_mid_str(0, CGR_ADM_DATA_NN, CGR_ADM_DATA_NN_LEN, 0, mid_str);
	adm_add_datadef("CGR_GET_ALL_CONTACTS",                  mid_str, 0,  adm_datalists_to_queue, adm_size_datalists);

	adm_build_mid_str(0, CGR_ADM_DATA_NN, CGR_ADM_DATA_NN_LEN, 1, mid_str);
	adm_add_datadef("CGR_GET_ALL_RANGES",                  mid_str, 0,  adm_datalists_to_queue, adm_size_datalists);


	/* Controls */
	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 0, mid_str);
	adm_add_ctrl("CGR_CONTACT_ADD", mid_str, 1); //from,to,startTime,endTime,xmitRate

	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 1, mid_str);
	adm_add_ctrl("CGR_CONTACT_REMOVE", mid_str, 1); //from,to,start

	adm_build_mid_str(0x41,CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 2, mid_str);
	adm_add_ctrl("CGR_RANGE_ADD", mid_str, 1); //to,from,via

	adm_build_mid_str(0x41, CGR_ADM_DATA_CTRL_NN, CGR_ADM_DATA_CTRL_NN_LEN, 3, mid_str);
	adm_add_ctrl("CGR_RANGE_REMOVE", mid_str, 1); //to,from

}


