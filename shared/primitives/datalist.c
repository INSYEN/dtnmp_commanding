#include "datalist.h"
/******************************************************************************
 *
 * \par Function Name: datalist_header_allocate
 *
 * \par Purpose: Allocates the items of a datalist_header_t. The header must already exist,
 *				This function just sets it up.
 * \return The number of items allocated
 *
 *
 * \param[in]   header	The header structure
 * \param[in]   dataSize The desired initial size.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

uint8_t datalist_header_allocate(datalist_header_t* header,uint8_t dataSize)
{
	//Sanity check
	if(header==NULL)
	{
		DTNMP_DEBUG_ERR("datalist_header_allocate","Header == null",NULL);
		return 0;
	}


	if(header->data!=NULL)
	{
		DTNMP_DEBUG_ERR("datalist_header_allocate","Header data != null",NULL);
		return 0;
	}


	header->data=(uint8_t*)STAKE(dataSize);
	header->length = dataSize;
	header->index=0;

	return dataSize;
}

/******************************************************************************
 *
 * \par Function Name: datalist_header_reallocate
 *
 * \par Purpose: Reallocates the items of a header_t, but only if the newSize is
 *		larger then the current size.
 * \return The number of items allocated
 *
 *
 * \param[in]   header	The header structure
 * \param[in]   newSize The desired initial size.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

uint8_t datalist_header_reallocate(datalist_header_t* header,uint8_t newSize)
{
	if(header==NULL)
		return 0;

	if(newSize<=header->length)
		return header->length;

	header->data=(uint8_t*)realloc(header->data,newSize);
	header->length = newSize;

	return newSize;
}

/******************************************************************************
 *
 * \par Function Name: datalist_create
 *
 * \par Purpose: Creates a datalist. If the datacol parameter is != null, it will
 *				Assume that is a semi-deserialized representation, and work accordingly.

 * \return The datalist
 *
 *
 * \param[in]   datacol	The datacol to deserialize from (optional)
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datalist_t datalist_create(Lyst* datacol=NULL)
{
	datalist_t newList;
	memset(&newList,0,sizeof(newList));
	if(datacol!=NULL)
	{
		newList.datacol=*datacol;


		//Deserialize header
		LystElt headerElt = lyst_first(newList.datacol);
		datacol_entry_t* headerCol = (datacol_entry_t*)lyst_data(headerElt);
		if((headerCol!=NULL) & (headerCol->length>0))
		{
			if(datalist_header_allocate(&newList.header,headerCol->length)!=0)
			{
				newList.header.index=headerCol->length;
				DTNMP_DEBUG_INFO("datalist_create","creating with nonnull datacollection size: %d",headerCol->length);

				memcpy(newList.header.data,headerCol->value,headerCol->length);

				return newList;
			}
			else
				DTNMP_DEBUG_ERR("datalist_create","Could not allocate header",NULL);
		}
		else
			DTNMP_DEBUG_ERR("datalist_create","HeaderCol is null or empty",NULL);
	}

	newList.datacol=lyst_create();
	datalist_header_allocate(&newList.header,8);

	return newList;
}

/******************************************************************************
 *
 * \par Function Name: datalist_get_type
 *
 * \par Purpose: returns the type of a given entry.
 * \return the requested type, or DLIST_INVALID.
 *
 *
 * \param[in]   datalist	The datalist
 * \param[in]   index		The zero-based index.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datalist_type_t datalist_get_type(datalist_t* datalist, uint8_t index)
{
	//Sanity check(s)
	if(index > datalist->header.length) //Index is obviously wrong, no need for subtle checks
		return DLIST_INVALID;

	//We're safe, cool.
	uint8_t* values=datalist->header.data;

	return (datalist_type_t)values[index];
}
/*
datalist_type_t datalist_get_info(datalist_t* datalist, uint8_t index)
{
	//Sanity check(s)
	if(index > datalist->header.length) //Index is obviously wrong, no need for subtle checks
		return DLIST_INVALID;

	//We're safe, cool.
	uint8_t* values=datalist->header.value;

	return (datalist_type_t)values[index] >> 3;

}
*/
/******************************************************************************
 *
 * \par Function Name: datalist_set_type
 *
 * \par Purpose: Sets the type for a given index, allocating additional header
 *				space as needed-
 * \return The type, or DLIST_INVALID if something failed.
 *
 *
 * \param[in]   datalist	The datalist
 * \param[in]   index		The zero-based index index
 * \param[in]   type		The requested type
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datalist_type_t datalist_set_type(datalist_t* datalist,uint8_t index,datalist_type_t type)
{

	//We can kill two birds with one stone, and just call the header reallocation function
	if(datalist_header_reallocate(&datalist->header,index)==0)
	{
		DTNMP_DEBUG_ERR("datalist_set_type","couldn't reallocate header",NULL);
		return DLIST_INVALID;
	}

	datalist->header.data[index]=type;

	return type;
}

/******************************************************************************
 *
 * \par Function Name: datalist_get_size_for_type
 *
 * \par Purpose: This is basically sizeof
 * \return The size, or 0 if A) something went wrong, or B) the variable is variable length
 *
 *
 * \param[in]   datalist	The datalist
 * \param[in]   index		The zero-based index index
 * \param[in]   type		The requested type
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

size_t datalist_get_size_for_type(datalist_type_t type)
{
	switch(type)
	{
		case DLIST_TYPE_INT32:
		case DLIST_TYPE_UINT32:
		case DLIST_TYPE_REAL32:
			return 4;
		break;

		case DLIST_TYPE_INT64:
		case DLIST_TYPE_UINT64:
		case DLIST_TYPE_REAL64:
			return 8;
		break;

		case DLIST_TYPE_UVAST:
		case DLIST_TYPE_VAST:
			return sizeof(uvast);
		break;

		case DLIST_TYPE_STRING:
		case DLIST_TYPE_BLOB:
		default:
			return 0;
	}
}

/******************************************************************************
 *
 * \par Function Name: datalist_get_colentry
 *
 * \par Purpose: Returns the datacollection entry for a given index
 * \return The datacol_entry_t, or NULL if something failed.
 *
 *
 * \param[in]   datalist	The datalist
 * \param[in]   index		The zero-based index index
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datacol_entry_t* datalist_get_colentry(datalist_t* datalist,uint8_t index)
{
	//sanity checks
	if(datalist==NULL)
		return NULL;

	if(datalist->header.length<index)
		return NULL;

	//Iterate through the list
	uint8_t idx = 0;
	LystElt elt;
	for(elt = lyst_first(datalist->datacol);idx!=index+1;elt=lyst_next(elt),idx++)
	{
		//noop, we're just using the lyst as an array here. Sorry Ed :(
	}
	//Alright, we're at the end
	if(elt)
		return (datacol_entry_t*)lyst_data(elt);

	return NULL;
}

/******************************************************************************
 *
 * \par Function Name: datalist_get
 *
 * \par Purpose: Grabs an item from the specified index, checks the type given is
 *				equal to the type of the item, then copies it to the given buffer.
 * \return The type, or DLIST_INVALID if something failed.
 *
 *
 * \param[in]   datalist	The datalist
 * \param[in]   index		The zero-based index index
 * \param[out]  optr		The buffer
 * \param[out]  outSize	The size of the item
 * \param[in]   type		The required type
 *
 * \par Notes: The item is copied to the given memory, so you must make sure that optr has enough space
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datalist_type_t datalist_get(datalist_t* datalist,uint8_t index,void* optr,size_t* outsize,datalist_type_t type)
{
	//sanity checks
	if(datalist==NULL)
		return DLIST_INVALID;

	if(datalist->header.length<index)
		return DLIST_INVALID;

	//Type checking
	if(datalist_get_type(datalist,index)!=type)
	{
		DTNMP_DEBUG_ERR("datalist_get","invalid type",NULL);
		return DLIST_INVALID;
	}
	//if(outsize<datalist_get_size_for_type(type))
	//	return DLIST_INVALID;

	//Ok, we should be good.
	datacol_entry_t* tempentry = datalist_get_colentry(datalist,index);

	//if(*outsize<tempentry->length)
	//	return DLIST_INVALID;
	//Deal with strings
	if(type==DLIST_TYPE_STRING)
	{
		DTNMP_DEBUG_INFO("datalist_get","Copying string of size",tempentry->length);
		memset(optr,'\0',tempentry->length+1);
	}

	if(tempentry!=NULL)
		memcpy(optr,tempentry->value,tempentry->length);
	else
		return DLIST_INVALID;


	if(outsize!=NULL)
		*outsize=tempentry->length;

	return datalist_get_type(datalist,index);
}

/******************************************************************************
 *
 * \par Function Name: datalist_insert_with_type
 *
 * \par Purpose: Inserts an item to the end of the datalist, taking into account its type. If the type
 *    			is variable length (blob, string), the size field must be filled out... memory is also
 *				allocated and the parameter copied, so remember to free your list!
 * \return The type inserted, or DLIST_INVALID if something failed.
 *
 *
 * \param[in]   datalist  A pointer to the lyst
 * \param[in]   type  	The type field
 * \param[in]   data	A pointer to the data which shall be inserted.
 * \param[in]   size	The size of the variable pointed to by data, required for variable length stuff.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datalist_type_t datalist_insert_with_type(datalist_t* datalist,datalist_type_t type,void* data,size_t size)
{
		//Step 0: Sanity check
		if((datalist==NULL) || (data==NULL))
			return DLIST_INVALID;
		//Step 1: Insert value
		datacol_entry_t* entry=(datacol_entry_t*)STAKE(sizeof(datacol_entry_t));
		size = size != 0 ? size : datalist_get_size_for_type(type);


		DTNMP_DEBUG_INFO("datalist_insert_with_type","Inserting chunk of size %d",size);
		entry->length=size;
		entry->value=(uint8_t*)STAKE(size);

		if(entry->value!=NULL)
			memcpy(entry->value,data,size);
		else
		{
			DTNMP_DEBUG_ERR("datalist_insert_with_type","Couldn't allocate ION working memory, this is really bad",NULL);
			return DLIST_INVALID;
		}
		//Step 2: Update header
		datalist_set_type(datalist,lyst_length(datalist->datacol),type);
		datalist->header.index++;
		//Insert
		lyst_insert_last(datalist->datacol,entry);
		return type;
}

/******************************************************************************
 *
 * \par Function Name: datalist_serialize_to_datacol
 *
 * \par Purpose: Serializes the entire datalist to a datacollection entry.
 * \return TA pointer to the datacol entry, or NULL if a failure occured.
 *
 *
 * \param[in]   datalist  A pointer to the lyst
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datacol_entry_t* datalist_serialize_to_datacol(datalist_t* datalist)
{
	datacol_entry_t* outDatacol = (datacol_entry_t*)STAKE(sizeof(datacol_entry_t));
	datacol_entry_t* header = (datacol_entry_t*)STAKE(sizeof(datacol_entry_t));
	Lyst tempBuffer=lyst_create();

	//Step 1: serialize the header
	header->value=(uint8_t*)STAKE(datalist->header.index);
	header->length=datalist->header.index;
	memcpy(header->value,datalist->header.data,header->length);

	lyst_insert_first(tempBuffer,header);

	//Step 2: Perform the copy
	for(LystElt elt = lyst_first(datalist->datacol); elt; elt = lyst_next(elt))
	{
		lyst_insert_last(tempBuffer,lyst_data(elt));
	}
	//Step 3: Serial the temp buffer
	outDatacol->value = utils_datacol_serialize(tempBuffer,&outDatacol->length);

	return outDatacol;
}

/******************************************************************************
 *
 * \par Function Name: datalist_deserialize_from_buffer
 *
 * \par Purpose: Deserializes a datalist from a byte array-
 * \return The datalist, or NULL if something failed.
 *
 *
 * \param[in]   buffer		the byte array
 * \param[in]   buffer_len 	The length of buffer
 * \param[out]  bufferOut	The length of buffer which was deserialized.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
datalist_t datalist_deserialize_from_buffer(uint8_t* buffer,uint32_t buffer_len,uint32_t* bufferOut)
{
	DTNMP_DEBUG_INFO("datalist_deserialize_from_buffer","buffer length = %u",buffer_len);
	Lyst dlCollection = utils_datacol_deserialize(buffer,buffer_len,bufferOut);
	datalist_t outDL = datalist_create(&dlCollection);

	DTNMP_DEBUG_INFO("datalist_deserialize_from_buffer","buffer length = %u",buffer_len);
	return outDL;
}
/******************************************************************************
 *
 * \par Function Name: datalist_get_size
 *
 * \par Purpose: Gets the size of a single item in the datalist.
 * \return the sizte, or NULL is something failed.
 *
 *
 * \param[in]   datalist  A pointer to the lyst
 * \param[in]   index	  The 0-based index of the item.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

size_t datalist_get_size(datalist_t* datalist,uint8_t index)
{
	if(datalist==NULL)
		return 0;

	if(datalist->header.length<index)
		return 0;

	//if(outsize<datalist_get_size_for_type(type))
	//	return DLIST_INVALID;

	//Ok, we should be good.
	datacol_entry_t* tempentry = datalist_get_colentry(datalist,index);

	if(tempentry==NULL)
	{
		DTNMP_DEBUG_INFO("datalist_get_size","Couldn't find element for index %d",index);
		return 0;
	}

	return tempentry->length;
}

/******************************************************************************
 *
 * \par Function Name: datalist_get_type_from_string
 *
 * \par Purpose: This is a utility function which returns the type (as a datalist_type_t) given a string
 *				representation. This is used in the RPC parser.
 * \return The type, or DLIST_INVALID if something failed.
 *
 *
 * \param[in]   string	The null-terminated string
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

datalist_type_t datalist_get_type_from_string(char* string)
{
	if(strcasecmp(string,"UINT32")==0)
		return DLIST_TYPE_UINT32;
	else if(strcasecmp(string,"INT32")==0)
		return DLIST_TYPE_INT32;
	else if(strcasecmp(string,"REAL32")==0)
		return DLIST_TYPE_REAL32;
	else if(strcasecmp(string,"REAL64")==0)
		return DLIST_TYPE_REAL64;
	else if(strcasecmp(string,"STRING")==0)
		return DLIST_TYPE_STRING;
	else if(strcasecmp(string,"UVAST")==0)
		return DLIST_TYPE_UVAST;
	else if(strcasecmp(string,"VAST")==0)
		return DLIST_TYPE_VAST;
	else if(strcasecmp(string,"UINT64")==0)
		return DLIST_TYPE_UINT64;
	else if(strcasecmp(string,"INT64")==0)
		return DLIST_TYPE_UINT64;
	//Welp, we've failed
	return DLIST_INVALID;
}

/******************************************************************************
 *
 * \par Function Name: datalist_get_string_from_type
 *
 * \par Purpose: This is a utility function which returns the type (as a string) given a type
 *				representation. This is used in the RPC parser.
 * \return The type, or "invalid" if something failed.
 *
 *
 * \param[in]   type		The type.
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

char* datalist_get_string_from_type(datalist_type_t type)
{
	switch (type)
	{
		case DLIST_TYPE_INT32:
			return "int32";
		case DLIST_TYPE_INT64:
			return "int64";
		case DLIST_TYPE_UINT32:
			return "uint32";
		case DLIST_TYPE_UINT64:
			return "uint64";
		case DLIST_TYPE_REAL32:
			return "real32";
		case DLIST_TYPE_REAL64:
			return "real64";
		case DLIST_TYPE_STRING:
			return "string";
		case DLIST_TYPE_UVAST:
			return "uvast";
		case DLIST_TYPE_VAST:
			return "vast";
	}
	return "invalid";
}

/******************************************************************************
 *
 * \par Function Name: datalist_printf_spec_from_type
 *
 * \par Purpose: This utility function creates a printf/scanf string given a type.
 * \return The representation, or "bs" if something failed.
 *
 *
 * \param[in]   type	The type field
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/

char* datalist_printf_spec_from_type(datalist_type_t type)
{
	switch (type)
	{
		case DLIST_TYPE_INT32:
		case DLIST_TYPE_INT64:
			return "%d";
		case DLIST_TYPE_UINT32:
			return "%u";
		case DLIST_TYPE_UINT64:
			return "%lu";
		case DLIST_TYPE_REAL32:
		case DLIST_TYPE_REAL64:
			return "%f";
		case DLIST_TYPE_STRING:
			return "%s";
		case DLIST_TYPE_UVAST:
			return UVAST_FIELDSPEC;
		case DLIST_TYPE_VAST:
			return VAST_FIELDSPEC;
	}
	return "bs";
}
/******************************************************************************
 *
 * \par Function Name: datalist_free_contents
 *
 * \par Purpose: Frees the datalist, including all contents within.
 * \return N/A
 *
 *
 * \param[in]   datalist  A pointer to the lyst
 *
 * \par Notes:
 *
 * Modification History:
 *  MM/DD/YY  AUTHOR         DESCRIPTION
 *  --------  ------------   ---------------------------------------------
 *  03/13/15  J.P Mayer      Initial implementation,
 *****************************************************************************/
void datalist_free_contents(datalist_t* datalist=NULL)
{
	//Sanity check
	if(datalist==NULL)
		return;
	//Free datacol
	utils_datacol_destroy(&datalist->datacol);

	//Free the header (if it exists)
	if(datalist->header.data!=NULL)
		SRELEASE(datalist->header.data);
}

uint8_t datalist_num_elements(datalist_t* datalist)
{
	//Sanity checks
	if(datalist==NULL)
		return 0;

	uint8_t tempSize = lyst_length(datalist->datacol);

	if(datalist->header.index==tempSize)
		return tempSize;
	else
		return 0;

}
