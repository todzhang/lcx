﻿// lcx.h: 标准系统包含文件的包含文件
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
#include "unisocket.h"

#include "../lib/wingetopt/src/getopt.h"
#include "../lib/libsocks/output-util.h"
#include "../lib/libsocks/client.h"
#include "../lib/libsocks/socks5-client.h"
#include "../lib/libsocks/socks5-server.h"


#define TIMEOUT 300
#define MAXSIZE 20480
#define HOSTLEN 40
#define CONNECTNUM 5

typedef enum _METHOD
{
	LISTEN = 10, TRAN, SLAVE, SSOCKSD=20, RCSOCKS, RSSOCKS, NETCAT=30
}METHOD;

#define STR_LISTEN "listen"
#define STR_TRAN "tran"
#define STR_SLAVE "slave"
#define STR_SSOCKSD "ssocksd"
#define STR_RCSOCKS "rcsocks"
#define STR_RSSOCKS "rssocks"
#define STR_NETCAT "netcat"

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

	int ssl;
}GlobalArgs;

extern GlobalArgs globalArgs;

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

void ssocksd(GlobalArgs args);
void rcsocks(GlobalArgs args);
void rssocks(GlobalArgs args);
int netcat(int argc, char** argv);
