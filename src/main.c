// main: 定义应用程序的入口点。
//

#include "lcx.h"

//#pragma comment(linker, "/FILEALIGN:32")
// 设置最小节的大小，数值越小文件的体积就越小，不过最小是16
//#pragma comment(linker, "/ALIGN:32")

// 清除从未引用的函数和/或数据
#pragma comment(linker, "/opt:ref")
#pragma comment (linker, "/OPT:ICF")
// 获得精简应用程序，减小体积
//#pragma comment(linker, "/opt:nowin98")


// 设置data段属性为可读可写可执行
//#pragma comment(linker, "/section:.data,RWE")
// 接下来就可以进行区段合并操作了比如，将上面的text和rdata都合并到data去
#pragma comment(linker, "/MERGE:.rdata=.data")
#pragma comment(linker, "/MERGE:.pdata=.data")
//#pragma comment(linker, "/MERGE:.idata=.data")
//#pragma comment(linker, "/MERGE:.text=.data")
//#pragma code_seg("section1")

   /* helpme :
	  the obvious */
static int helpme()
{
	printf("lcx v" VERSION "\n");
	printf("./lcx ([-options][values])* \n\
	options :\n\
	- S state setup the function.You can pick one from the following options :\n\
	ssocksd, rcsocks, rssocks, listen, tran, slave, netcat\n\
	- l listenport open a port for the service startup.\n\
	- d refhost set the reflection host address.\n\
	- e refport set the reflection port.\n\
	- f connhost set the connect host address .\n\
	- g connport set the connect port.\n\
	- h help show the help text, By adding the - s parameter, you can also see the more detailed help.\n\
	- a about show the about pages\n\
	- v version show the version.\n\
	- t usectime set the milliseconds for timeout.The default value is 1000");
	return(0);
} /* helpme */

METHOD str2method(char* method) {
	if (!strcmp(method, STR_LISTEN)) {
		return LISTEN;
	}
	else if (!strcmp(method, STR_TRAN)) {
		return TRAN;
	}
	else if (!strcmp(method, STR_SLAVE)) {
		return SLAVE;
	}
	else if (!strcmp(method, STR_SSOCKSD)) {
		return SSOCKSD;
	}
	else if (!strcmp(method, STR_RCSOCKS)) {
		return RCSOCKS;
	}
	else if (!strcmp(method, STR_RSSOCKS)) {
		return RSSOCKS;
	}
	else if (!strcmp(method, STR_NETCAT)) {
		return NETCAT;
	}
	else {
		return 0;
	}
}

GlobalArgs globalArgs;

int main(int argc, char* argv[])
{
	/* Initialize globalArgs before we get to work. */
	memset(&globalArgs, 0, sizeof(globalArgs));

	/* getopt_long stores the option index here. */
	int option_index = 0;

	register int option;
	while ((option = getopt(argc, argv, ":S:l:m:d:e:f:g:v:")) != EOF) {

		switch (option) {
		case 'S':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.method = str2method(optarg);
			}
			if (globalArgs.method == NETCAT) {
				goto  argsFinished;
			}
			break;

		case 'l':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.iListenPort = atoi(optarg);
			}
			break;

		case 'm':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.iTransmitPort = atoi(optarg);
			}
			break;

		case 'd':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.connectHost = optarg;
			}
			break;

		case 'e':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.iConnectPort = atoi(optarg);
			}
			break;

		case 'f':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.transmitHost = optarg;
			}
			break;

		case 'g':
			printf("Given Option: %c\n", option);
			if (optarg) {
				globalArgs.iTransmitPort = atoi(optarg);
			}
			break;

		case 'v':
			printf("Given Option: %c\n", option);
			globalArgs.verbosity = 1;
			break;
		case '?':
			printf("Given Option: %c\n", option);
		default:
			errno = 0;
			helpme();
		} /* switch x */
	} /* while getopt */
	//globalArgs.inputFiles = argv + optind;
	//globalArgs.numInputFiles = argc - optind;
argsFinished:;

	// Win Start Winsock.
	WSADATA wsadata;
	WSAStartup(MAKEWORD(1, 1), &wsadata);

#ifdef WIN32
	WSADATA wsaData;
	int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaInit != 0) {
		ERROR(L_NOTICE, "WSAStartup failed: %d\n", wsaInit);
		exit(1);
	}
#endif

	signal(SIGINT, &getctrlc);

	if (globalArgs.bFreeConsole)
	{
		FreeConsole();
	}

	switch (globalArgs.method)
	{
	case LISTEN:
		bind2bind(globalArgs);
		break;
	case TRAN:
		bind2conn(globalArgs);
		break;
	case SLAVE:
		conn2conn(globalArgs);
		break;

	case SSOCKSD:
		ssocksd(globalArgs);
		break;
	case RCSOCKS:
		rcsocks(globalArgs);
		break;
	case RSSOCKS:
		rssocks(globalArgs);
		break;
	case NETCAT:
		netcat(argc, argv);
		break;
	default:
		helpme();
		break;
	}

	if (globalArgs.method) // cleanup
	{
		closeallfd();
	}

#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}