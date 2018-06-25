#ifndef VARIABLES_H_INCLUDED
#define VARIABLES_H_INCLUDED
#include "shared/utils/nm_types.h"
/**
* Data Structure for variables to be printed/sent.
* The hope here is that we won't have to have a bunch of domain-specific print functions
* Added by Jeremy Pierce Mayer
**/
typedef enum
{
	TYPE_UINT32,
	TYPE_INT32,
	TYPE_STRING,
	TYPE_DATALIST,
	TYPE_UVAST,
	TYPE_VAST,
} variableType;

typedef struct
{
	eid_t producer_eid;
	uint8_t producerSource;
	/**
	* Contains variable type
	**/
	variableType type;
	time_t timestamp; //Potentially unused, but useful for reporting.
	char* name;
	uint8_t* value;
	size_t size;
} variableQueueEntry;

variableQueueEntry AddVariable(char* name, variableType type, void* value,
    eid_t* sourceEid, size_t size, time_t timestamp);


#endif // VARIABLES_H_INCLUDED
