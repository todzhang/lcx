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

#include "../lib/getopt/getopt.h"


#define VERSION "1.00"
#define TIMEOUT 300
#define MAXSIZE 20480
#define HOSTLEN 40
#define CONNECTNUM 5

// define 2 socket struct
typedef struct _transocket
{
	SOCKET fd1;
	SOCKET fd2;
}transocket;

// define function 
void ver();
void usage(char* prog);
void transmitdata(LPVOID data);
void getctrlc(int j);
void closeallfd();
void makelog(char* buffer, int length);
void proxy(int port);
void bind2bind(int port1, int port2);
void bind2conn(int port1, char* host, int port2);
void conn2conn(char* host1, int port1, char* host2, int port2);
int testifisvalue(char* str);
int create_socket();
int create_server(int sockfd, int port);
int client_connect(int sockfd, char* server, int port);
