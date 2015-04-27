#include "shared/primitives/variables.h"
#include "shared/utils/utils.h"
variableQueueEntry AddVariable(char* name,variableType type, void* value,eid_t* sourceEid,size_t size, time_t timestamp)
{
	variableQueueEntry tempEntry;
	switch(type)
	{
		case TYPE_UINT32:
		case TYPE_INT32:
			size=4;
		break;
		case TYPE_UVAST:
		case TYPE_VAST:
			size=8;
		case TYPE_STRING:
			size=strlen((char*)value);
		case TYPE_DATALIST:
			if(size==0)
				break;
	}
	if(size==0)
	{
		DTNMP_DEBUG_ERR("addvar","value size = 0",NULL);
		value=NULL;
	}

	size_t nameLen = strlen(name)+1;
	tempEntry.value=(uint8_t*)STAKE(size);
	tempEntry.name=(char*)STAKE(nameLen);
	strncpy(tempEntry.name,name,nameLen);
	memcpy(tempEntry.value,value,size);

	tempEntry.size=size;
	tempEntry.type=type;

	if(sourceEid==NULL)
	{
		strcpy(&tempEntry.producer_eid.name[0],"None");
	}
	else
	{
		memcpy(&tempEntry.producer_eid,sourceEid,sizeof(eid_t));
	}
	return tempEntry;
}
