#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "hstring.h"
#include "DecoderInterface.h"
#include "DecoderConstants.h"

#define HEADER_SIZE 6
#define _DEBUG
#ifdef _DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

bool ParseDecoder(unsigned char *data , int size)
{
	HSTRING *CommandString;
	int numUsed = 0;
	int posOfSemi = 0;
	int posOfStart = 0;
	const unsigned char header[]="DECSET"; 
	
	debug("data is %s,size is:%d\r\n",data,size);
	
	CommandString = new HSTRING( data, size, false, true, true );
	
	if(( size == 0 )||(NULL == data))
		return false;

	CommandString->FindString(header,(unsigned int)size,false,1,&posOfStart);
	posOfStart = posOfStart+HEADER_SIZE;
	debug("posOfStart:%d\r\n",posOfStart);
	
	CommandString->FindCharacterForward( ':',0,&posOfSemi );
	debug("posOfSemi:%d\r\n",posOfSemi);
	
	if( posOfSemi != CommandString->Size ) {
		int tag = CommandString->ConvertHexToNumber(posOfStart,posOfSemi,&numUsed);
		if( numUsed < 8 )
			tag += MENU_TYPE_INT;
		int data = CommandString->ConvertToNumber(posOfSemi+1,&numUsed);
		if( numUsed ) {
			if(0 < DecodeSet(tag,(void *)data))
			/*if(0 < DecodeSet( 0x1a02a001, (void *)0 ))*/
				debug("Decoder setting is ok, tag is: %x ; data is: %x\n",tag,data);
			else
				printf("configure failed!!!!!!!!!!\r\n");
			//DecodeGet( tag, &testData );
			//debug("get data is: %x\n",testData);
			
		} else 
			debug("Decoder setting is fail, tag is: %x ; data is: %x\n",tag,data);
	} else {
		printf("error!,could not find :\n");
		int tag = CommandString->ConvertHexToNumber(posOfStart,&numUsed);
		if( numUsed < 8 )
			tag += MENU_TYPE_INT;
		int tmpBuffer[25];
		tmpBuffer[0] = -9999;
		DecodeGet(tag,(void *)tmpBuffer);
	}

	delete CommandString;
	return true;
}

