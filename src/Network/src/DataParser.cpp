#include "DataParser.h"
#include "hstring.h"
#include "DecoderInterface.h"
#include "DecoderConstants.h"
#include "logging.h"
#include <string.h>
#include <chrono>
#include <thread>

#define _DEBUG
#ifdef _DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#define LOG_TAG "DataPraser"
#define LOG_LEVEL LOG_PRINT //directly print in console
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

#define SYN 0X16
#define DC1 0X11

extern int StartSendingImageStream(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket);
extern int StopSendingImageStream(EventManager *event_manager);
extern int StartSendDecoderData(ScanDriver *scan_driver, EventManager *event_manager,TcpServerSocket *tcp_socket);
extern int StopSendDecoderData(EventManager *event_manager);


DataParser::DataParser(READER_CONFIGURATION *pRc,ScanDriver *pSc,EventManager *pEv,TcpServerSocket *pTs):
	pReaderConfiguration(pRc),pScanDriver(pSc),pEventManager(pEv),pTcpSocket(pTs)
{
    //pReaderConfiguration = NULL;
}

DataParser::~DataParser()
{

}

//PDCFEX1200.
//upcena1.
//USRCMD0ffff0001:1200;
//DECSET1a013001:1;.

int DataParser::ProcessData(HSTRING str)
{
    if(NULL == pReaderConfiguration)
        return -1;
    //HSTRING str(buf,len);
    debug("hstring is %s\r\n",str.Char);
    //std::cout<<"str is"<<str<<std::endl;
    int ret = pReaderConfiguration->Menu(&str);
    debug("ret is :%d\n",ret);
    return ret;
}

int DataParser::ProcessData(char *buf, int len)
{
	LOGGING("Enter ProcessData\r\n");

    //printf("[DataParser]:Call ProcessData\r\n");
    if((NULL == pReaderConfiguration)/*||(NULL == buf)*/)
        return -1;
	LOGGING("Enter ProcessData2\r\n");
    //HSTRING str(buf,len);
    //printf("[DataParser]:hstring is:%s\r\n",str.Char);
    //return ProcessData(str);
	if((buf[0] == 'D')&&(buf[1] == 'E')) {
		printf("[DataParser]:ParseDecoder\r\n");
		ParseDecoder((unsigned char *)buf,len);	
	} else if ((buf[0] == 'U')&&(buf[1] == 'S')) {
		printf("[DataParser]:ParseSensorSettingData\r\n");
		ParseSensorSettingData((unsigned char *)buf,len);		
	} else {
		HSTRING str(buf,len);
		printf("[DataParser]:hstring is:%s\r\n",str.Char);
		int ret = pReaderConfiguration->Menu(&str);
    	printf("ret is :%d\n",ret);
	}
	printf("[DataParser]:exit ProcessData\r\n");
	return true;
}

int DataParser::ParseDecoder(unsigned char *data , int size)
{
    #define HEADER_SIZE 6
    HSTRING *CommandString;
    int numUsed = 0;
    int posOfSemi = 0;
    int posOfStart = 0;
    const unsigned char header[]="DECSET";

    debug("data is %s,size is:%d\r\n",data,size);

    if(( size == 0 )||(NULL == data))
        return false;
	
	if(NULL == pReaderConfiguration)
		return false;
	
    CommandString = new HSTRING( data, size, false, true, true );	
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

int DataParser::ParseSensorSettingData(unsigned char *buffer , int size)
{
    #define HEADER_SIZE 6
    HSTRING *CommandString;
    int numUsed = 0;
    int posOfSemi = 0;
    int posOfStart = 0;
    const unsigned char header[] = "USRCMD";

    debug("[DataParser]:str is %s,size is:%d\r\n",buffer,size);

    if(( size == 0 )||(NULL == buffer))
        return false;
	
	if(NULL == pReaderConfiguration)
		return false;	
	
    CommandString = new HSTRING( buffer, size, false, true, true );

    CommandString->FindString(header,(unsigned int)size,false,1,&posOfStart);
    posOfStart = posOfStart + HEADER_SIZE;
    debug("[DataParser]:posOfStart:%d\r\n",posOfStart);

    CommandString->FindCharacterForward( ':',0,&posOfSemi );
    debug("[DataParser]:posOfSemi:%d\r\n",posOfSemi);

    if( posOfSemi != CommandString->Size ) {
        int tag = CommandString->ConvertHexToNumber(posOfStart,posOfSemi,&numUsed);
        //if( numUsed < 8 )
            //tag += MENU_TYPE_INT;
        int data = CommandString->ConvertToNumber(posOfSemi+1,&numUsed);
	
		delete CommandString;
		
		debug("[DataParser]:Tag:%x Data:%d\r\n",tag,data);
        if( numUsed ) {
			switch (tag) {
				case 0x0ffff001: {//ALL ENABLE
					if(data == 0) {
						debug("[DataParser]:Disbale all symbologies\n");
						HSTRING str3("ALLENA0!",true);
						//int ret1 = 0;
						int ret1 = pReaderConfiguration->Menu(&str3);
						//std::this_thread::sleep_for(std::chrono::microseconds(1000));
    					debug("[DataParser]:Set Menu ret is :%d\n",ret1);
					} else if(data == 1) {
						debug("[DataParser]:Enable all symbologies\n");
						HSTRING str4("ALLENA1!",true);
						//int ret2 = 0;
						int ret2 = pReaderConfiguration->Menu(&str4);
						//std::this_thread::sleep_for(std::chrono::microseconds(1000));
    					debug("[DataParser]:Set Menu ret is :%d\n",ret2);
					}
					break;
				}
				case 0x0ffff002: {//PDCFEX
						debug("[DataParser]:Set sensor expsoure!\n");
						char buf1[100];
						sprintf(buf1,"PDCFEX%d.",data);
						printf("menu is %s\r\n",buf1);
						HSTRING str1(buf1,strlen(buf1));
						int ret3 = pReaderConfiguration->Menu(&str1);
						debug("[DataParser]:Set Sensor ret is :%d\n",ret3);
					break;
				}
				case 0x0ffff003: {//PDCFGX
					debug("[DataParser]:Set sensor gain!\n");
					char buf2[100];
					sprintf(buf2,"PDCFGX%d.",data);
					printf("menu is %s\r\n",buf2);
					HSTRING str2(buf2,strlen(buf2));
					int ret4 = pReaderConfiguration->Menu(&str2);
					debug("[DataParser]:Set Sensor ret is :%d\n",ret4);
					break;
				}
				default:
					break;
			}
        } else
            debug("setting is fail, tag is: %x ; data is: %x\n",tag,data);
    } else {
    	delete CommandString;
        printf("error!,could not find :\n");
    }
	
    //delete CommandString;
    printf("[DataParser]:exit ParseSensorSettingData\r\n");
    return true;
}

