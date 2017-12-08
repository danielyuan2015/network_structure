#ifndef DATAPARSER_H
#define DATAPARSER_H

#include "ReaderConfiguration.h"
#include "hstring.h"

class DataParser
{
public:
    DataParser(READER_CONFIGURATION *pRc);
    int ProcessData(HSTRING str);
    int ProcessData(char buf,int len);

private:
    READER_CONFIGURATION *pReaderConfiguration;

};

#endif // DATAPARSER_H
