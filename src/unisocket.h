#ifndef UNISOCK
#define UNISOCK

#ifdef _WIN32
#define CLOSE_SOCKET closesocket
#define CLOSE_AND_CLEAN(x) closesocket(x); WSACleanup()
#else
#define CLOSE_SOCKET close
#define CLOSE_AND_CLEAN close
#endif

#ifdef _WIN32
#include <winsock.h>
#include <stdint.h>
#define sockaddr_un sockaddr_in
#pragma comment(lib, "ws2_32.lib")
# define sleep(_x)		Sleep((_x)*1000)
#else
#include <unistd.h>     /* close */
#include <sys/wait.h>   /* wait */
#include <sys/socket.h> /* socket, bind, sendto, recvfrom, getsockname */
#include <sys/un.h>     /* socket domaine AF_UNIX */
#include <netinet/ip.h> /* socket domaine AF_INET */
#include <arpa/inet.h>  /* inet_ntoa */
#include <netdb.h>      /* gethostbyname */
#include <sys/time.h>   /* gettimeofday */
#endif

#endif
