// lcx.cpp: 定义应用程序的入口点。
//

#include "lcx.h"

#ifdef DEBUG
#define Debug(x) printf x; printf ("\n"); fflush (stdout); sleep (1);
#else
#define Debug(x)	/* nil... */
#endif

   /* helpme :
	  the obvious */
static int helpme()
{
	printf("lcx v" VERSION "\n");
    printf("./lcx ([-options][values])* \n\
	options :\n\
	- s state setup the function.You can pick one from the following options :\n\
	ssocksd, rcsocks, rssocks, lcx_listen, lcx_tran, lcx_slave\n\
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
	//globalArgs.method = 0;     /* false */
	//globalArgs.verbosity = 0;
	//globalArgs.connectHost = NULL;
	//globalArgs.transmitHost = NULL;
	//globalArgs.bFreeConsole = 0;
	//globalArgs.ssl = 0;

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
				globalArgs.transmitHost = atoi(optarg);
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
argsFinished: ;

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

//************************************************************************************
//
// LocalHost:ConnectPort transmit to LocalHost:TransmitPort
//
//************************************************************************************
void bind2bind(GlobalArgs args)
{
	SOCKET fd1, fd2, sockfd1, sockfd2;
	struct sockaddr_in client1, client2;
	int size1, size2;

	HANDLE hThread = NULL;
	transocket sock;
	DWORD dwThreadID;

	if ((fd1 = create_socket()) == 0) return;
	if ((fd2 = create_socket()) == 0) return;

	int port1 = args.iListenPort;
	int port2 = args.iConnectPort;

	printf("[+] Listening port %d ......\r\n", port1);
	fflush(stdout);

	if (create_server(fd1, port1) == 0)
	{
		closesocket(fd1);
		return;
	}

	printf("[+] Listen OK!\r\n");
	printf("[+] Listening port %d ......\r\n", port2);
	fflush(stdout);
	if (create_server(fd2, port2) == 0)
	{
		closesocket(fd2);
		return;
	}

	printf("[+] Listen OK!\r\n");
	size1 = size2 = sizeof(struct sockaddr);
	while (1)
	{
		printf("[+] Waiting for Client on port:%d ......\r\n", port1);
		if ((sockfd1 = accept(fd1, (struct sockaddr*) & client1, &size1)) < 0)
		{
			printf("[-] Accept1 error.\r\n");
			continue;
		}

		printf("[+] Accept a Client on port %d from %s ......\r\n", port1, inet_ntoa(client1.sin_addr));
		printf("[+] Waiting another Client on port:%d....\r\n", port2);
		if ((sockfd2 = accept(fd2, (struct sockaddr*) & client2, &size2)) < 0)
		{
			printf("[-] Accept2 error.\r\n");
			closesocket(sockfd1);
			continue;
		}

		printf("[+] Accept a Client on port %d from %s\r\n", port2, inet_ntoa(client2.sin_addr));
		printf("[+] Accept Connect OK!\r\n");

		sock.fd1 = sockfd1;
		sock.fd2 = sockfd2;

		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transmitdata, (LPVOID)&sock, 0, &dwThreadID);
		if (hThread == NULL)
		{
			TerminateThread(hThread, 0);
			return;
		}

		Sleep(1000);
		printf("[+] CreateThread OK!\r\n\n");
	}
}

//************************************************************************************
// 
// LocalHost:ConnectPort transmit to TransmitHost:TransmitPort
//
//************************************************************************************
void bind2conn(GlobalArgs args)
{
	int port1 = args.iListenPort;
	char* host = args.transmitHost;
	int port2 = args.iTransmitPort;

	SOCKET sockfd, sockfd1, sockfd2;
	struct sockaddr_in remote;
	int size;
	char buffer[1024];

	HANDLE hThread = NULL;
	transocket sock;
	DWORD dwThreadID;

	if (port1 > 65535 || port1 < 1)
	{
		printf("[-] ConnectPort invalid.\r\n");
		return;
	}

	if (port2 > 65535 || port2 < 1)
	{
		printf("[-] TransmitPort invalid.\r\n");
		return;
	}

	memset(buffer, 0, 1024);
	if ((sockfd = create_socket()) == INVALID_SOCKET) return;

	if (create_server(sockfd, port1) == 0)
	{
		closesocket(sockfd);
		return;
	}

	size = sizeof(struct sockaddr);
	while (1)
	{
		printf("[+] Waiting for Client ......\r\n");
		if ((sockfd1 = accept(sockfd, (struct sockaddr*) & remote, &size)) < 0)
		{
			printf("[-] Accept error.\r\n");
			continue;
		}

		printf("[+] Accept a Client from %s:%d ......\r\n",
			inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
		if ((sockfd2 = create_socket()) == 0)
		{
			closesocket(sockfd1);
			continue;
		}
		printf("[+] Make a Connection to %s:%d ......\r\n", host, port2);
		fflush(stdout);

		if (client_connect(sockfd2, host, port2) == 0)
		{
			closesocket(sockfd2);
			sprintf(buffer, "[SERVER]connection to %s:%d error\r\n", host, port2);
			send(sockfd1, buffer, strlen(buffer), 0);
			memset(buffer, 0, 1024);
			closesocket(sockfd1);
			continue;
		}

		printf("[+] Connect OK!\r\n");

		sock.fd1 = sockfd1;
		sock.fd2 = sockfd2;

		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transmitdata, (LPVOID)&sock, 0, &dwThreadID);
		if (hThread == NULL)
		{
			TerminateThread(hThread, 0);
			return;
		}

		Sleep(1000);
		printf("[+] CreateThread OK!\r\n\n");
	}
}

//************************************************************************************
// 
// ConnectHost:ConnectPort transmit to TransmitHost:TransmitPort
//
//************************************************************************************
void conn2conn(GlobalArgs args)
{
	
	char* host1 = args.connectHost;
	int port1 = args.iConnectPort;
	char* host2 = args.transmitHost;
	int port2 = args.iTransmitPort;

	SOCKET sockfd1, sockfd2;

	HANDLE hThread = NULL;
	transocket sock;
	DWORD dwThreadID;
	fd_set fds;
	int l;
	char buffer[MAXSIZE];

	while (1)
	{
		if ((sockfd1 = create_socket()) == 0) return;
		if ((sockfd2 = create_socket()) == 0) return;

		printf("[+] Make a Connection to %s:%d....\r\n", host1, port1);
		fflush(stdout);
		if (client_connect(sockfd1, host1, port1) == 0)
		{
			closesocket(sockfd1);
			closesocket(sockfd2);
			continue;
		}

		// fix by bkbll 
		// if host1:port1 recved data, than connect to host2,port2
		l = 0;
		memset(buffer, 0, MAXSIZE);
		while (1)
		{
			FD_ZERO(&fds);
			FD_SET(sockfd1, &fds);
			if (select(sockfd1 + 1, &fds, NULL, NULL, NULL) == SOCKET_ERROR)
			{
				if (errno == WSAEINTR) continue;
				break;
			}
			if (FD_ISSET(sockfd1, &fds))
			{
				l = recv(sockfd1, buffer, MAXSIZE, 0);
				break;
			}
			Sleep(5);
		}

		if (l <= 0)
		{
			printf("[-] There is a error...Create a new connection.\r\n");
			continue;
		}
		while (1)
		{
			printf("[+] Connect OK!\r\n");
			printf("[+] Make a Connection to %s:%d....\r\n", host2, port2);
			fflush(stdout);
			if (client_connect(sockfd2, host2, port2) == 0)
			{
				closesocket(sockfd1);
				closesocket(sockfd2);
				continue;
			}

			if (send(sockfd2, buffer, l, 0) == SOCKET_ERROR)
			{
				printf("[-] Send failed.\r\n");
				continue;
			}

			l = 0;
			memset(buffer, 0, MAXSIZE);
			break;
		}

		printf("[+] All Connect OK!\r\n");

		sock.fd1 = sockfd1;
		sock.fd2 = sockfd2;

		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transmitdata, (LPVOID)&sock, 0, &dwThreadID);
		if (hThread == NULL)
		{
			TerminateThread(hThread, 0);
			return;
		}

		// connectnum++;

		Sleep(1000);
		printf("[+] CreateThread OK!\r\n\n");
	}
}

//************************************************************************************
//
// Socket Transmit to Socket
//
//************************************************************************************
void transmitdata(LPVOID data)
{
    SOCKET fd1, fd2;
    transocket* sock;
    struct timeval timeset;
    fd_set readfd, writefd;
    int result, i = 0;
    char read_in1[MAXSIZE], send_out1[MAXSIZE];
    char read_in2[MAXSIZE], send_out2[MAXSIZE];
    int read1 = 0, totalread1 = 0, send1 = 0;
    int read2 = 0, totalread2 = 0, send2 = 0;
    int sendcount1, sendcount2;
    int maxfd;
    struct sockaddr_in client1, client2;
    int structsize1, structsize2;
    char host1[20], host2[20];
    int port1 = 0, port2 = 0;
    char tmpbuf[100];

    sock = (transocket*)data;
    fd1 = sock->fd1;
    fd2 = sock->fd2;

    memset(host1, 0, 20);
    memset(host2, 0, 20);
    memset(tmpbuf, 0, 100);

    structsize1 = sizeof(struct sockaddr);
    structsize2 = sizeof(struct sockaddr);

    if (getpeername(fd1, (struct sockaddr*) & client1, &structsize1) < 0)
    {
        strcpy(host1, "fd1");
    }
    else
    {
        //            printf("[+]got, ip:%s, port:%d\r\n",inet_ntoa(client1.sin_addr),ntohs(client1.sin_port));
        strcpy(host1, inet_ntoa(client1.sin_addr));
        port1 = ntohs(client1.sin_port);
    }

    if (getpeername(fd2, (struct sockaddr*) & client2, &structsize2) < 0)
    {
        strcpy(host2, "fd2");
    }
    else
    {
        //            printf("[+]got, ip:%s, port:%d\r\n",inet_ntoa(client2.sin_addr),ntohs(client2.sin_port));
        strcpy(host2, inet_ntoa(client2.sin_addr));
        port2 = ntohs(client2.sin_port);
    }

    printf("[+] Start Transmit (%s:%d <-> %s:%d) ......\r\n\n", host1, port1, host2, port2);

    maxfd = max(fd1, fd2) + 1;
    memset(read_in1, 0, MAXSIZE);
    memset(read_in2, 0, MAXSIZE);
    memset(send_out1, 0, MAXSIZE);
    memset(send_out2, 0, MAXSIZE);

    timeset.tv_sec = TIMEOUT;
    timeset.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);

        FD_SET((UINT)fd1, &readfd);
        FD_SET((UINT)fd1, &writefd);
        FD_SET((UINT)fd2, &writefd);
        FD_SET((UINT)fd2, &readfd);

        result = select(maxfd, &readfd, &writefd, NULL, &timeset);
        if ((result < 0) && (errno != EINTR))
        {
            printf("[-] Select error.\r\n");
            break;
        }
        else if (result == 0)
        {
            printf("[-] Socket time out.\r\n");
            break;
        }

        if (FD_ISSET(fd1, &readfd))
        {
            /* must < MAXSIZE-totalread1, otherwise send_out1 will flow */
            if (totalread1 < MAXSIZE)
            {
                read1 = recv(fd1, read_in1, MAXSIZE - totalread1, 0);
                if ((read1 == SOCKET_ERROR) || (read1 == 0))
                {
                    printf("[-] Read fd1 data error,maybe close?\r\n");
                    break;
                }

                memcpy(send_out1 + totalread1, read_in1, read1);
                sprintf(tmpbuf, "\r\nRecv %5d bytes from %s:%d\r\n", read1, host1, port1);
                printf(" Recv %5d bytes %16s:%d\r\n", read1, host1, port1);
                makelog(tmpbuf, strlen(tmpbuf));
                makelog(read_in1, read1);
                totalread1 += read1;
                memset(read_in1, 0, MAXSIZE);
            }
        }

        if (FD_ISSET(fd2, &writefd))
        {
            int err = 0;
            sendcount1 = 0;
            while (totalread1 > 0)
            {
                send1 = send(fd2, send_out1 + sendcount1, totalread1, 0);
                if (send1 == 0)break;
                if ((send1 < 0) && (errno != EINTR))
                {
                    printf("[-] Send to fd2 unknow error.\r\n");
                    err = 1;
                    break;
                }

                if ((send1 < 0) && (errno == ENOSPC)) break;
                sendcount1 += send1;
                totalread1 -= send1;

                printf(" Send %5d bytes %16s:%d\r\n", send1, host2, port2);
            }

            if (err == 1) break;
            if ((totalread1 > 0) && (sendcount1 > 0))
            {
                /* move not sended data to start addr */
                memcpy(send_out1, send_out1 + sendcount1, totalread1);
                memset(send_out1 + totalread1, 0, MAXSIZE - totalread1);
            }
            else
                memset(send_out1, 0, MAXSIZE);
        }

        if (FD_ISSET(fd2, &readfd))
        {
            if (totalread2 < MAXSIZE)
            {
                read2 = recv(fd2, read_in2, MAXSIZE - totalread2, 0);
                if (read2 == 0)break;
                if ((read2 < 0) && (errno != EINTR))
                {
                    printf("[-] Read fd2 data error,maybe close?\r\n\r\n");
                    break;
                }

                memcpy(send_out2 + totalread2, read_in2, read2);
                sprintf(tmpbuf, "\r\nRecv %5d bytes from %s:%d\r\n", read2, host2, port2);
                printf(" Recv %5d bytes %16s:%d\r\n", read2, host2, port2);
                makelog(tmpbuf, strlen(tmpbuf));
                makelog(read_in2, read2);
                totalread2 += read2;
                memset(read_in2, 0, MAXSIZE);
            }
        }

        if (FD_ISSET(fd1, &writefd))
        {
            int err2 = 0;
            sendcount2 = 0;
            while (totalread2 > 0)
            {
                send2 = send(fd1, send_out2 + sendcount2, totalread2, 0);
                if (send2 == 0)break;
                if ((send2 < 0) && (errno != EINTR))
                {
                    printf("[-] Send to fd1 unknow error.\r\n");
                    err2 = 1;
                    break;
                }
                if ((send2 < 0) && (errno == ENOSPC)) break;
                sendcount2 += send2;
                totalread2 -= send2;

                printf(" Send %5d bytes %16s:%d\r\n", send2, host1, port1);
            }
            if (err2 == 1) break;
            if ((totalread2 > 0) && (sendcount2 > 0))
            {
                /* move not sended data to start addr */
                memcpy(send_out2, send_out2 + sendcount2, totalread2);
                memset(send_out2 + totalread2, 0, MAXSIZE - totalread2);
            }
            else
                memset(send_out2, 0, MAXSIZE);
        }

        Sleep(5);
    }

    closesocket(fd1);
    closesocket(fd2);

    printf("\r\n[+] OK! I Closed The Two Socket.\r\n");
}

void closeallfd()
{
    int i;

    printf("[+] Let me exit ......\r\n");
    fflush(stdout);

    for (i = 3; i < 256; i++)
    {
        closesocket(i);
    }

    FILE* fp = globalArgs.logFile;
    if (fp != NULL)
    {
        fprintf(fp, "\r\n====== Exit ======\r\n");
        fclose(fp);
    }

    printf("[+] All Right!\r\n");
}

int create_socket()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[-] Create socket error.\r\n");
        return(0);
    }

    return(sockfd);
}

int create_server(int sockfd, int port)
{
    struct sockaddr_in srvaddr;
    int on = 1;

    memset(&srvaddr, 0, sizeof(struct sockaddr));

    srvaddr.sin_port = htons(port);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)); //so I can rebind the port

    if (bind(sockfd, (struct sockaddr*) & srvaddr, sizeof(struct sockaddr)) < 0)
    {
        printf("[-] Socket bind error.\r\n");
        return(0);
    }

    if (listen(sockfd, CONNECTNUM) < 0)
    {
        printf("[-] Socket Listen error.\r\n");
        return(0);
    }

    return(1);
}

int client_connect(int sockfd, char* server, int port)
{
	struct sockaddr_in cliaddr;
	struct hostent* host;

	if (!(host = gethostbyname(server)))
	{
		printf("[-] Gethostbyname(%s) error:%s\n", server, strerror(errno));
		return(0);
	}

	memset(&cliaddr, 0, sizeof(struct sockaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(port);
	cliaddr.sin_addr = *((struct in_addr*)host->h_addr);

	if (connect(sockfd, (struct sockaddr*) & cliaddr, sizeof(struct sockaddr)) < 0)
	{
		printf("[-] Connect error.\r\n");
		return(0);
	}
	return(1);
}

void makelog(char* buffer, int length)
{
    FILE* fp = globalArgs.logFile;
    if (fp != NULL)
    {
        //            fprintf(fp, "%s", buffer);
        //            printf("%s",buffer);
        write(fileno(fp), buffer, length);
        //            fflush(fp);
    }
}

void getctrlc(int j)
{
    printf("\r\n[-] Received Ctrl+C\r\n");
    closeallfd();
    exit(0);
}

#define DEFAULT_PORT 1080

// global to prevent messing with the stack
// see http://stackoverflow.com/questions/1847789/segmentation-fault-on-large-array-sizes
s_client tc[MAXCLI];

int boucle_princ = 1;
void capte_fin(int sig) {
	printf("serveur: signal %d caught\n", sig);
	boucle_princ = 0;
}

// ssocksd
void ssocksd(GlobalArgs args) {
	const char* bindAddr =args.listenHost;
	int port = args.iListenPort;
	int ssl = args.ssl;
	int soc_ec = -1, maxfd, res, nc;
	fd_set set_read;
	fd_set set_write;
	struct sockaddr_in addrS;
	char methods[2];

	s_socks_conf conf;
	s_socks_server_config config;
	conf.config.srv = &config;

	char versions[] = { SOCKS5_V,
						SOCKS4_V
	};

	config.allowed_version = versions;
	config.n_allowed_version = sizeof(versions);

	methods[0] = 0x00;

	config.allowed_method = methods;
	config.n_allowed_method = 1;
	//config.check_auth = check_auth;

	/* Init client tab */
	for (nc = 0; nc < MAXCLI; nc++)
		init_client(&tc[nc], nc, M_SERVER, &conf);

	if (bindAddr == 0)
		soc_ec = new_listen_socket(NULL, port, 0, &addrS);
	else
		soc_ec = new_listen_socket(bindAddr, port, 0, &addrS);

	if (soc_ec < 0) goto fin_serveur;


#ifndef WIN32
	if (globalArgsServer.daemon == 1) {
		TRACE(L_NOTICE, "server: mode daemon ...");
		if (daemon(0, 0) != 0) {
			perror("daemon");
			exit(1);
		}
		writePID(PID_FILE);
	}

	bor_signal(SIGINT, capte_fin, SA_RESTART);

	/* Need in daemon to remove the PID file properly */
	bor_signal(SIGTERM, capte_fin, SA_RESTART);
	bor_signal(SIGPIPE, capte_sigpipe, SA_RESTART);
	/* TODO: Find a better way to exit the select and recall the init_select
	 * SIGUSR1 is send by a thread to unblock the select */
	bor_signal(SIGUSR1, capte_usr1, SA_RESTART);
#endif

	while (boucle_princ) {
		init_select_server(soc_ec, tc, &maxfd, &set_read, &set_write);

		res = select(maxfd + 1, &set_read, &set_write, NULL, NULL);

		if (res > 0) { /* Search eligible sockets */

			if (FD_ISSET(soc_ec, &set_read))
				new_connection(soc_ec, tc, ssl);

			for (nc = 0; nc < MAXCLI; nc++) {
				dispatch_server(&tc[nc], &set_read, &set_write);
			}

		}
		else if (res == 0) {

		}
		else if (res < 0) {
			if (errno == EINTR); /* Received signal, it does nothing */
			else {
				perror("select");
				goto fin_serveur;
			}
		}
	}

fin_serveur:
#ifdef HAVE_LIBSSL
	if (globalArgsServer.ssl == 1)
		ssl_cleaning();
#endif
	TRACE(L_NOTICE, "server: closing sockets ...");
	close_log();
	for (nc = 0; nc < MAXCLI; nc++) disconnection(&tc[nc]);
	if (soc_ec != -1) CLOSE_SOCKET(soc_ec);
}



//#define PORT 1080

// global to prevent messing with the stack
// see http://stackoverflow.com/questions/1847789/segmentation-fault-on-large-array-sizes
s_socket socks_pool[MAXCLI];
//s_client tc[MAXCLI];

void new_connection_reverse(int soc_ec, s_client* tc, s_socket* socks_pool)
{
	int nc, nc2, soc_tmp;
	struct sockaddr_in adrC_tmp;

	//memset(&adrC_tmp, 0, sizeof(struct sockaddr_in));

	TRACE(L_DEBUG, "server: connection in progress (reverse) ...");
	soc_tmp = bor_accept_in(soc_ec, &adrC_tmp);
	if (soc_tmp < 0) {
		return;
	}

	/* Search free space in tc[].soc */
	for (nc = 0; nc < MAXCLI; nc++)
		if (tc[nc].soc.soc == -1) break;

	/* Search for a relay in socks_pool */
	for (nc2 = 0; nc2 < MAXCLI; nc2++)
		if (socks_pool[nc2].soc != -1) break;

	if (nc < MAXCLI && nc2 < MAXCLI) {
		init_client(&tc[nc], tc[nc].id, tc[nc].socks.mode, tc[nc].conf);
		tc[nc].soc.soc = soc_tmp;
		tc[nc].socks.state = S_REPLY;
		tc[nc].socks.connected = 1;

		memcpy(&tc[nc].soc_stream, &socks_pool[nc2], sizeof(s_socks));

		/* Remove from the pool */
		socks_pool[nc2].soc = -1;

		memcpy(&tc[nc].soc.adrC, &adrC_tmp, sizeof(struct sockaddr_in));
		TRACE(L_VERBOSE, "server [%d]: established connection with %s",
			nc, bor_adrtoa_in(&adrC_tmp));

		//append_log_client(&tc[nc], "%s", bor_adrtoa_in(&adrC_tmp));
		//set_non_blocking(tc[nc].soc);
	}
	else {
		CLOSE_SOCKET(soc_tmp);
		//ERROR(L_NOTICE, "server: %s connection refused : too many clients!",
		//	bor_adrtoa_in(&adrC_tmp));
	}

}

void new_connection_socket(int soc_ec, s_socket* tc, int ssl)
{
	int nc, soc_tmp;
	struct sockaddr_in adrC_tmp;

	TRACE(L_DEBUG, "server: connection server in progress (socket) ...");
	soc_tmp = bor_accept_in(soc_ec, &adrC_tmp);
	if (soc_tmp < 0) {
		return;
	}

	/* Search free space in tc[].soc */
	for (nc = 0; nc < MAXCLI; nc++)
		if (tc[nc].soc == -1) break;

	if (nc < MAXCLI) {
		init_socket(&tc[nc]);

		tc[nc].soc = soc_tmp;
		memcpy(&tc[nc].adrC, &adrC_tmp, sizeof(struct sockaddr_in));
		TRACE(L_VERBOSE, "server [%d]: established server connection with %s",
			nc, bor_adrtoa_in(&adrC_tmp));

#ifdef HAVE_LIBSSL
		/* Init SSL here
		 */
		if (ssl == 1) {
			TRACE(L_DEBUG, "server [%d]: socks5 enable ssl  ...", nc);
			tc[nc].ssl = ssl_neogiciate_server(tc[nc].soc);
			if (tc[nc].ssl == NULL) {
				ERROR(L_VERBOSE, "server [%d]: ssl error", nc);
				close_socket(&tc[nc]);
				return;
			}
			TRACE(L_DEBUG, "server [%d]: ssl ok.", nc);
			set_non_blocking(tc[nc].soc);
		}
#endif /* HAVE_LIBSSL */

		//append_log_client(&tc[nc], "%s", bor_adrtoa_in(&adrC_tmp));
		//set_non_blocking(tc[nc].soc);
	}
	else {
		CLOSE_SOCKET(soc_tmp);
		ERROR(L_NOTICE, "server: %s connection refused : too many clients!",
			bor_adrtoa_in(&adrC_tmp));
	}
}

void init_select_reverse(int soc_ec, int soc_ec_cli, s_client* tc, int* maxfd,
	fd_set* set_read, fd_set* set_write)
{
	int nc;
	/* TODO: move FD_ZERO */
	FD_ZERO(set_read);
	FD_ZERO(set_write);

	FD_SET(soc_ec, set_read);
	FD_SET(soc_ec_cli, set_read);

	*maxfd = soc_ec_cli;
	for (nc = 0; nc < MAXCLI; nc++) {
		s_client* client = &tc[nc];

		init_select_server_cli(&client->soc, &client->socks,
			&client->buf, &client->stream_buf, maxfd, set_read, set_write);

		init_select_server_stream(&client->soc_stream, &client->socks,
			&client->stream_buf, &client->buf, maxfd, set_read, set_write);


		if (client->soc_bind.soc != -1) {
			FD_SET(client->soc_bind.soc, set_read);
			if (client->soc_bind.soc > * maxfd) *maxfd = client->soc_bind.soc;
		}
	}
}

// server_relay
void rcsocks(GlobalArgs args) {
	int port = args.iConnectPort;
	int listen = args.iListenPort;
	int ssl = args.ssl;

	int soc_ec_cli = -1, soc_ec = -1, maxfd, res, nc;
	fd_set set_read;
	fd_set set_write;
	struct sockaddr_in addrS;

	/* Init client tab */
	for (nc = 0; nc < MAXCLI; nc++)
		init_socket(&socks_pool[nc]);

	for (nc = 0; nc < MAXCLI; nc++)
		init_client(&tc[nc], nc, 0, NULL);

	TRACE(L_NOTICE, "server: set listening client socks relay ...");
	soc_ec = new_listen_socket(NULL, listen, MAXCLI, &addrS);
	if (soc_ec < 0) goto fin_serveur;

	TRACE(L_NOTICE, "server: set server relay ...");
	soc_ec_cli = new_listen_socket(NULL, port, MAXCLI, &addrS);
	if (soc_ec_cli < 0) goto fin_serveur;


#ifndef WIN32
	if (globalArgs.background == 1) {
		TRACE(L_NOTICE, "server: background ...");
		if (daemon(0, 0) != 0) {
			perror("daemon");
			exit(1);
		}
	}

	bor_signal(SIGINT, capte_fin, SA_RESTART);

	/* TODO: Find a better way to exit the select and recall the init_select
	 * SIGUSR1 is send by a thread to unblock the select */
	bor_signal(SIGUSR1, capte_usr1, SA_RESTART);
#endif

	boucle_princ = 1;
	while (boucle_princ) {
		init_select_reverse(soc_ec, soc_ec_cli, tc, &maxfd, &set_read, &set_write);

		res = select(maxfd + 1, &set_read, &set_write, NULL, NULL);

		if (res > 0) {  /* Search eligible sockets */

			if (FD_ISSET(soc_ec, &set_read))
				new_connection_socket(soc_ec, socks_pool, ssl);

			if (FD_ISSET(soc_ec_cli, &set_read))
				new_connection_reverse(soc_ec_cli, tc, socks_pool);

			for (nc = 0; nc < MAXCLI; nc++) {
				dispatch_server(&tc[nc], &set_read, &set_write);
			}
		}
		else if (res == 0) {
			/* If timeout was set in select and expired */
		}
		else if (res < 0) {
			if (errno == EINTR);  /* Received signal, it does nothing */
			else {
				perror("select");
				goto fin_serveur;
			}
		}
	}

fin_serveur:
#ifdef HAVE_LIBSSL
	if (ssl == 1)
		ssl_cleaning();
#endif
	printf("Server: closing sockets ...\n");
	if (soc_ec != -1) CLOSE_SOCKET(soc_ec);
	for (nc = 0; nc < MAXCLI; nc++) close_socket(&socks_pool[nc]);
	for (nc = 0; nc < MAXCLI; nc++) disconnection(&tc[nc]);

}

// reverse_server
void rssocks(GlobalArgs args) {
	char* sockshost = args.connectHost;
	int socksport = args.iConnectPort;
	char* uname = NULL;
	char* passwd= NULL;
	int ssl = args.ssl;
	int ncon = 25;
	int soc_ec = -1, maxfd, res, nc, k;
	fd_set set_read;
	fd_set set_write;

	s_socks_conf conf;
	s_socks_client_config config_cli;
	s_socks_server_config config_srv;

	conf.config.cli = &config_cli;
	conf.config.srv = &config_srv;

	char method[] = { 0x00, 0x02 };
	char version[] = { SOCKS5_V };
	conf.config.srv->n_allowed_version = 1;
	conf.config.srv->allowed_version = version;
	conf.config.srv->n_allowed_method = 1;
	conf.config.srv->allowed_method = method;


	conf.config.cli->n_allowed_method = 2;
	conf.config.cli->allowed_method = method;

	/* If no username or password  we don't use auth */
	if (uname == NULL || passwd == NULL)
		--conf.config.cli->n_allowed_method;

	conf.config.cli->loop = 1;
	conf.config.cli->host = NULL;
	conf.config.cli->port = 0;
	conf.config.cli->sockshost = sockshost;
	conf.config.cli->socksport = socksport;
	conf.config.cli->username = uname;
	conf.config.cli->password = passwd;
	conf.config.cli->version = version[0];

	/* Init client tab */
	for (nc = 0; nc < MAXCLI; nc++) init_client(&tc[nc], nc, M_SERVER, &conf);


#ifndef WIN32
	if (globalArgs.background == 1) {
		TRACE(L_NOTICE, "server: background ...");
		if (daemon(0, 0) != 0) {
			perror("daemon");
			exit(1);
		}
	}

	bor_signal(SIGINT, capte_fin, SA_RESTART);
#endif

	boucle_princ = 1;
	while (boucle_princ) {
		k = init_select_server_reverse(tc, &maxfd, ncon,
			&set_read, &set_write, ssl);
		if (k < 0) {
			break;
		}
		res = select(maxfd + 1, &set_read, &set_write, NULL, NULL);

		if (res > 0) { /* Search eligible sockets */
			for (nc = 0; nc < MAXCLI; nc++) {
				k = dispatch_server(&tc[nc], &set_read, &set_write);
				if (k == -2) {
					// Not sure why this was set to exit the loop; much more stable without it
					//boucle_princ = 0;
					break;
				}
			}
		}
		else if (res == 0) {

		}
		else if (res < 0) {
			if (errno == EINTR); /* Received signal, it does nothing */
			else {
				perror("select");
				break;
			}
		}
	}

#ifdef HAVE_LIBSSL
	if (ssl == 1)
		ssl_cleaning();
#endif
	printf("Server: closing sockets ...\n");
	if (soc_ec != -1) CLOSE_SOCKET(soc_ec);
	for (nc = 0; nc < MAXCLI; nc++) disconnection(&tc[nc]);

}
