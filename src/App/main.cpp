#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "socket.h"
#include "logging.h"
#include "Network.h"
//#include "ipc.h"
#include <iostream>
#include <string>  

#define LOG_TAG "main"
#define LOG_LEVEL LOG_PRINT //directly print in console
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

using namespace std;
extern IPC *gIpcServerPtr;
extern IpcClient *gIpcClientPtr;

int main(int argc, char **argv)
{
	string str;
	char c;
	char buf2[]={"hello world!"};
	//char buf2[]={0x01,0x02,0x03};
	
	ScanDriver *psd = new ScanDriver();
	EventManager *pem = new EventManager();
	READER_CONFIGURATION *prc = new READER_CONFIGURATION();
	//std::cout<<"hello world!"<<std::endl;
	LOGGING("hello world\r\n");
	//int StartNetworkServer(int tcpPort,int udpPort,ScanDriver *scan_driver, EventManager *event_manager,READER_CONFIGURATION *pRConfg)
	InitNetworkServer(55266,55266,psd,pem,prc);
	
	cout<<"please type test command:"<<endl;
	cout<<"1 - connect ipc class"<<endl;
	cout<<"0 - disconnect ipc class"<<endl;
	cout<<"3 - start network class"<<endl;
	cout<<"4 - stop network class"<<endl;
	
	while((c=cin.get())!=EOF) {
		cout.put(c);
		switch(c){
			case '0':{
				gIpcClientPtr->TryDisconnect();
				break;
			}
			case '1':{
				gIpcClientPtr->TryConnect();
				break;
			}
			case '2':{
				LOGGING("choose 2\r\n");
				gIpcServerPtr->IpcWrite(buf2,sizeof(buf2));
				break;
			}
			case '3':{
				StartNetworkServer();
				break;
			}
			case '4':{
				StopNetworkServer();
				break;
			}
			case '5':{
				//gIpcServerPtr->SocketWrite(buf2,sizeof(buf2));;
				break;
			}
			default:
			//LOGGING("not support this commad\r\n");
			break;
		}
	}

	/*while (getline(cin, str)) {
       //LOGGING("%s\r\n");
	   cout << str << endl;
    }*/
	return 0;
}
