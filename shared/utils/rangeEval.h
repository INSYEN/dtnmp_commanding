#ifndef RANGEEVAL_H_INCLUDED
#define RANGEEVAL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	OP_UPRANGE=0x01,
	OP_MT=0x02,
	OP_LOWRANGE=0x03,
	OP_LT=0x04,
	OP_NOT=0x05,
	OP_EQ=0x06,

} operatorsT;
typedef struct
{
	operatorsT op;
	unsigned long val;
} dataT;
typedef void (*rangeCallback)(unsigned int,void*);

dataT GenerateOperation(char* input) //Iterate backwards, generate first operation
{
	size_t x=0;
	dataT curData;
	curData.op=0;
	curData.val=0;
	if(input==NULL)
	{
		curData.op=OP_EQ;
		curData.val=0;

		return curData;

	}
	if(strlen(input)==0)
	{
		curData.op=OP_EQ;
		curData.val=0;

		return curData;
	}
	////printf("DEBUG: \"%s\"  %d\n",input,strlen(input));
	for(x=strlen(input);x!=-1;x--)
	{
	//	//printf("%d\n",x);
		switch(input[x])
		{
			case '>': //MT/upper range operation
	//		//printf("DEEEBUGGG: >\n");
				if(input[x-1]=='>')
				{
				//	x++;
					//printf("l\n");
					curData.op=OP_LOWRANGE;
					break;
				}
				else
					curData.op=OP_MT;
			break;

		//	break;

			case '<': //MT/upper range operation
	//			//printf("DEEEBUGGG: <\n");
				if(input[x-1]=='<')
				{
				//	x++;
					curData.op=OP_UPRANGE;
					break;
				}
				else
					curData.op=OP_LT;
			break;
			//break;
			case '\!':
				//printf("DEEEBUGGG:! =  %c %d\n",input[x],x);
				curData.op=OP_NOT;
				break;

			case ' ': //Cheap way to skip whitespace
				x--;
				break;
			//default: //Implicit = operation
		//	GenerateOperation((char*)&input[x+1]);


			//	curData.val=atoi((const char*)&input[x]);

		}
		if(curData.op)
		{
			//printf("curData Break %d\n",curData.op);
			break;
		}

	}
	if(!curData.op)
	{
		curData.op=OP_EQ;
		//printf("OPEQ\n");
	}

	curData.val=atoi((const char*)&input[x+1]);
//	printf("=%d",curData.val);
	return curData;
}
typedef struct
{
	enum
	{
		ST_OPHUNTING,
		ST_ITERATING,
		ST_END,
	} state;
	int lowRange;
	int highRange;
	int counter;
	char* tokReentry;

} stateT;

stateT* CreateState()
{
	stateT* state=malloc(sizeof(stateT));

	state->lowRange=-1;
	state->highRange=-1;
	state->counter=-1;

	return state;
}

short StateMachine(dataT* curOp,stateT* state,char** tokReentry,rangeCallback callback,void* userData)
{
	switch(state->state)
	{
		case ST_ITERATING:
		//printf("st_iterating\n");
			if(state->counter==-1)
			{
				state->counter=state->lowRange;
				//printf("starting\n");

				return 1;
			}

			if(state->counter>state->highRange)
			{
				//printf("counterHigh %d\n",state->highRange);
				state->state=ST_OPHUNTING;
				return 0;
			}

		case ST_OPHUNTING:
			switch (curOp->op)
			{
				case OP_LOWRANGE:
				//	printf("lr %d\n",curOp->val);
					state->lowRange=curOp->val;
					state->highRange=-1;
					state->counter=-1;

					return 0;

				break;
				case OP_UPRANGE:

					state->highRange=curOp->val;
				//	printf("ur %d",state->highRange);
					if(state->lowRange==-1)
						return 0;
					else
					if(curOp->op!=OP_EQ)
					{
						free(state->tokReentry);
						state->tokReentry=malloc(strlen(*tokReentry));
						strcpy(state->tokReentry,*tokReentry);
					//	printf("configuring backtrack %s\n",state->tokReentry);
					}
						state->state=ST_ITERATING;
					//	printf("DEBUG!!!!!!!!!! starting");

						return 0;
				break;
				case OP_NOT: //Not only works for iterators

					//curOp->op=OP_EQ;
					if(state->state==ST_ITERATING)
					{
					//	printf("op_not = %d %d\n",state->counter,curOp->val);
						if(state->counter==curOp->val)
						{

							state->counter++;
							return 0;
						}

					//	else
					//		return 0;
					}
					else
						return 0;
				break;
				default: //Once again, implicit equals operation
				case OP_EQ:
				//	printf("opEq\n");

					if(state->state==ST_ITERATING) //We have an active iterator, backtrack
					{
							callback(state->counter,userData);
							state->counter++;
						//	printf("backtracking %s\n",state->tokReentry);
						//	*tokReentry=state->tokReentry;
							strcpy(*tokReentry,state->tokReentry);
							return 0;
					}
					else
					{
						state->state==ST_OPHUNTING;

						state->counter=curOp->val;
						callback(state->counter,userData);
						//printf("returning\n");
					}
			}
	}

	return 0;
}

void ParseRanges(char* input,rangeCallback callback,void* userData)
{
	if(callback==NULL)
		return;

	char* opsTerm=input;

	//printf("ot: %s\n",argv[1]);
	char *curTok,*lastTok;
	stateT* state=CreateState();
	for(curTok=opsTerm;;opsTerm=NULL)
	{
		if(curTok!=NULL)
		{
			curTok=strtok_r(opsTerm,",",&lastTok);

		}

		if((curTok==NULL)&(state->state!=ST_ITERATING))
			return 0;

		//Now do the processing
		dataT curOp = GenerateOperation((curTok));

		//State machine for token
		for(;StateMachine(&curOp,state,&lastTok,callback,userData);)
		{
		//	//
		}



	}

    return 0;
}
#endif // RANGEEVAL_H_INCLUDED
