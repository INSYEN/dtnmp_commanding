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
 ** File Name: adm_bp.h
 **
 ** Description: This file contains the definitions of the CGR
 **              ADM.
 **
 ** Notes:
 **
 ** Assumptions:
 **      1. We current use a non-official OID root tree for DTN Bundle Protocol
 **         identifiers.
 **
 **
 ** Modification History:
 **  MM/DD/YY  AUTHOR         DESCRIPTION
 **  --------  ------------   ---------------------------------------------
 **  03/09/2015 Jeremy		  Initial version
 *****************************************************************************/

#ifndef _HAVE_CGR_ADM_
#define _HAVE_CGR_ADM_

#include "lyst.h"
//#include "bpnm.h"

#include "shared/utils/nm_types.h"


#include "shared/adm/adm.h"




/*
 * [3] arrays ar eby classes of service.
 * 0 - BULK
 * 1 - NORM
 * 2 - EXP
 */


/*
 * +--------------------------------------------------------------------------+
 * |						      ADM CONSTANTS  							  +
 * +--------------------------------------------------------------------------+
 */


/*
 * We will invent an OID space for BP ADM information, to live at:
 *
 * iso.identified-organization.dod.internet.mgmt.dtnmp.bp
 * or 1.3.6.1.2.3.1
 * or, as OID,: 2A 06 01 02 03 01
 *
 * Note: dtnmp.bp is a made-up subtree.
 */


static char* CGR_ADM_ROOT = "2B0601020305";
#define BP_ADM_ROOT_LEN (6)

/*
 * +--------------------------------------------------------------------------+
 * |					  ADM ATOMIC DATA DEFINITIONS  						  +
 * +--------------------------------------------------------------------------+
 */


/*
 * Structure: Node Info is subtree at 01. Endpoint is subtree at 02. CLA is
 *            subtree at 03.
 *
 *                   ADM_BP_ROOT (2A0601020301)
 *                        |
 *      NODE_INFO (01)    |    ENDPOINT_INFO (02)       CTRL (03)
 *           +------------+----------+---------------------+
 *           |            |          |                     |
 */

/* Node-Specific Definitions */
static char* CGR_ADM_DATA_NN = "2B060102030501";
#define CGR_ADM_DATA_NN_LEN  (7)


/* Endpoint-Specific Definitions */
static char* CGR_ADM_DATA_END_NN = "2B060102030502";
#define CGR_ADM_DATA_END_NN_LEN  (7)


/* Bundle Protocol Controls */
static char* CGR_ADM_DATA_CTRL_NN = "2B060102030503";
#define CGR_ADM_DATA_CTRL_NN_LEN  (7)



/*
 * +--------------------------------------------------------------------------+
 * |					        FUNCTION PROTOTYPES  						  +
 * +--------------------------------------------------------------------------+
 */

void adm_cgr_init();

/* Custom Print Functions */

/* Custom Size Functions. */



#endif //_HAVE_CGR_ADM_
