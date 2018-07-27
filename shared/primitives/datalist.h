#ifndef DATALIST_H_INCLUDED
#define DATALIST_H_INCLUDED
#include "lyst.h"
#include "shared/utils/utils.h"
#include "shared/utils/nm_types.h"


typedef enum
{
DLIST_TYPE_INT32  = 0,
DLIST_TYPE_UINT32 = 1,
DLIST_TYPE_VAST   = 2,
DLIST_TYPE_UVAST  = 3,
DLIST_TYPE_REAL32 = 4,
DLIST_TYPE_REAL64 = 5,
DLIST_TYPE_BLOB   = 6,
DLIST_TYPE_STRING = 7,
DLIST_TYPE_INT64  = 8,
DLIST_TYPE_UINT64 = 9,
DLIST_INVALID	  = 10,
} datalist_type_t;
typedef struct
{
	uint8_t length; /*Max current length*/
	uint8_t index; /*current 0-based index*/
	uint8_t* data;
} datalist_header_t;
typedef struct
{
	Lyst datacol;
	datalist_header_t header;
} datalist_t;
/**************Defines*************/
#define CHKVALID(x) {if (x==DLIST_INVALID) {DTNMP_DEBUG_ERR("datalist","invalid datalist, exiting",NULL);return 0;}}
/**************Functions***********/
uint8_t datalist_header_reallocate(datalist_header_t* header,uint8_t newSize);
datalist_t datalist_create(Lyst* datacol);
datalist_type_t datalist_get_type(datalist_t* datalist, uint8_t index);
size_t datalist_get_size_for_type(datalist_type_t type);
//datalist_type_t datalist_get_info(datalist_t* datalist, uint8_t index);
datalist_type_t datalist_set_type(datalist_t* datalist,uint8_t index,datalist_type_t type);
datacol_entry_t* datalist_get_colentry(datalist_t* datalist,uint8_t index);
datalist_type_t datalist_get(datalist_t* datalist,uint8_t index,void* optr,size_t* outsize,datalist_type_t type);
uint8_t datalist_header_allocate(datalist_header_t* header,uint8_t dataSize);
datalist_type_t datalist_insert_with_type(datalist_t* datalist,datalist_type_t type,void* data,size_t size=0);
datacol_entry_t* datalist_serialize_to_datacol(datalist_t* datalist);
size_t datalist_get_size(datalist_t* datalist,uint8_t index);
datalist_t datalist_deserialize_from_buffer(uint8_t* buffer,uint32_t buffer_len,uint32_t* bufferOut);
char* datalist_printf_spec_from_type(datalist_type_t type);
datalist_type_t datalist_get_type_from_string(char* string);
void datalist_free_contents(datalist_t* datalist);
uint8_t datalist_num_elements(datalist_t* datalist);
char* datalist_get_string_from_type(datalist_type_t type);
#endif // DATALIST_H_INCLUDED
