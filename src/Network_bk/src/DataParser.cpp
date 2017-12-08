#include "DataParser.h"
#include "hstring.h"

DataParser::DataParser(READER_CONFIGURATION *pRc):pReaderConfiguration(pRc)
{
    pReaderConfiguration = NULL;
}

int DataParser::ProcessData(HSTRING str)
{
    if(NULL == pReaderConfiguration)
        return -1;
    //HSTRING str(buf,len);
    printf("hstring is %s\r\n",str.Char);
    //std::cout<<"str is"<<str<<std::endl;
    int ret = pReaderConfiguration->Menu(&str);
    printf("ret is :%d\n",ret);
    return ret;
}

int DataParser::ProcessData(char buf, int len)
{
    if(NULL == pReaderConfiguration)
        return -1;
    HSTRING str(buf,len);
    return ProcessData(str);
}
