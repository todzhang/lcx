// lcx.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

//ss#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <signal.h>
#include <errno.h>
#include <io.h> 

#include <math.h>

#include <sys/types.h> 

#include "LcxConfig.h"
#include "socket.h"

#include "../lib/wingetopt/src/getopt.h"


#define TIMEOUT 300
#define MAXSIZE 20480
#define HOSTLEN 40
#define CONNECTNUM 5

typedef enum _METHOD
{
	LISTEN = 1, TRAN, SLAVE
}METHOD;

#define STR_LISTEN "listen"
#define STR_TRAN "tran"
#define STR_SLAVE "slave"

// define 2 socket struct
typedef struct _transocket
{
	SOCKET fd1;
	SOCKET fd2;
}transocket;


typedef struct _GlobalArgs {
	METHOD method;                /* -methon option */
	char* logFileName;    /* -o option */
	FILE* logFile;
	int verbosity;              /* -v option */

	int iListenPort;
	char* listenHost;

	int iConnectPort;
	char* connectHost;

	int iTransmitPort;
	char* transmitHost;

	int bFreeConsole;
}GlobalArgs;

// define function 
void ver();
void usage(char* prog);
void transmitdata(LPVOID data);
void getctrlc(int j);
void closeallfd();
void makelog(char* buffer, int length);
void proxy(int port);
void bind2bind(GlobalArgs args);
void bind2conn(GlobalArgs args);
void conn2conn(GlobalArgs args);
int testifisvalue(char* str);
int create_socket();
int create_server(int sockfd, int port);
int client_connect(int sockfd, char* server, int port);


METHOD str2method(char* method);