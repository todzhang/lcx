#if defined(WIN32) 
	#include <winsock.h>
	#pragma comment(lib, "ws2_32.lib")
	# define sleep(_x)		Sleep((_x)*1000)
#else
	#include <sys/socket.h> 
	#include <netinet/in.h> 
	#include <arpa/inet.h> 
	#include <unistd.h> 
	#include <sys/time.h> 
	#include <netdb.h> 
#endif