#ifndef DATAPARSER_H
#define DATAPARSER_H

#include "ReaderConfiguration.h"
#include "ScanDriver.h"
#include "hstring.h"
#include "TcpServerSocket.h"

class TcpServerSocket;

class DataParser
{
public:
    DataParser(READER_CONFIGURATION *pRc,ScanDriver *pSc,EventManager *pEv,TcpServerSocket *pTs);
    virtual ~DataParser();
    int ParseDecoder(unsigned char *data , int size);
	int ParseSensorSettingData(unsigned char *data , int size);
    int ProcessData(HSTRING str);
    int ProcessData(char *buf,int len);

private:
    READER_CONFIGURATION *pReaderConfiguration = NULL;
	ScanDriver *pScanDriver = NULL;
	EventManager *pEventManager = NULL;
	TcpServerSocket *pTcpSocket = NULL;

};

#endif // DATAPARSER_H
