/*****************************************************************************
 **
 ** File Name: adm_bp_priv.h
 **
 ** Description: This implements the private aspects of a BP ADM.
 **
 ** Notes:
 **
 ** Assumptions:
 **
 **
 ** Modification History:
 **  MM/DD/YY  AUTHOR         DESCRIPTION
 **  --------  ------------   ---------------------------------------------
 **  02/25/15  E. Birrane     Initial Implementation
 *****************************************************************************/
#ifndef ADM_CGR_PRIV_H_
#define ADM_CGR_PRIV_H_


#include "shared/adm/adm_cgr.h"
#include "shared/utils/expr.h"
#include "shared/primitives/datalist.h"

void agent_adm_init_cgr();

/******************************************************************************
 *                            Retrieval Functions                             *
 ******************************************************************************/
expr_result_t cgr_node_get_contacts(Lyst params);
expr_result_t cgr_node_get_ranges(Lyst params);
/******************************************************************************
 *                              Control Functions                             *
 ******************************************************************************/

uint32_t cgr_ctrl_contact_add(Lyst params);
uint32_t cgr_ctrl_contact_remove(Lyst params);
uint32_t cgr_ctrl_range_add(Lyst params);
uint32_t cgr_ctrl_range_remove(Lyst params);

#endif //#ifndef ADM_CGR_PRIV_H_


