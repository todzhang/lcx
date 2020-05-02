// lcx.cpp: 定义应用程序的入口点。
//

#include "lcx.h"


/* Debug macro: squirt whatever to stderr and sleep a bit so we can see it go
   by.  need to call like Debug ((stuff)) [with no ; ] so macro args match!
   Beware: writes to stdOUT... */
#ifdef DEBUG
#define Debug(x) printf x; printf ("\n"); fflush (stdout); sleep (1);
#else
#define Debug(x)	/* nil... */
#endif



/* helpme :
   the obvious */
static int helpme()
{
	printf("NetCat for Windows v" VERSION " https://github.com/diegocr/netcat\n\
connect to somewhere:	nc [-options] hostname port[s] [ports] ... \n\
listen for inbound:	nc -l -p port [options] [hostname] [port]\n\
options:");
	printf("\
	-d		detach from console, background mode");

	printf("\
	-e prog		inbound program to exec [dangerous!!]");
	printf("\
	-g gateway	source-routing hop point[s], up to 8\n\
	-G num		source-routing pointer: 4, 8, 12, ...\n\
	-h		this cruft\n\
	-i secs		delay interval for lines sent, ports scanned\n\
	-l		listen mode, for inbound connects\n\
	-L		listen harder, re-listen on socket close\n\
	-n		numeric-only IP addresses, no DNS\n\
	-o file		hex dump of traffic\n\
	-p port		local port number\n\
	-r		randomize local and remote ports\n\
	-s addr		local source address");
	printf("\
	-u		UDP mode\n\
	-v		verbose [use twice to be more verbose]\n\
	-w secs		timeout for connects and final net reads\n\
	-x		handle ansi escape codes\n\
	-z		zero-I/O mode [used for scanning]");
	return(0);
} /* helpme */


struct globalArgs_t {
	int noIndex;                /* -I option */
	char* langCode;             /* -l option */
	const char* outFileName;    /* -o option */
	FILE* outFile;
	int verbosity;              /* -v option */
	char** inputFiles;          /* input files */
	int numInputFiles;          /* # of input files */
} globalArgs;

static const char* optString = "adg:G:hi:lLno:p:rs:tuvw:zx";

int main(int argc, char* argv[])
{
	/* Initialize globalArgs before we get to work. */
	globalArgs.noIndex = 0;     /* false */
	globalArgs.langCode = NULL;
	globalArgs.outFileName = NULL;
	globalArgs.outFile = NULL;
	globalArgs.verbosity = 0;
	globalArgs.inputFiles = NULL;
	globalArgs.numInputFiles = 0;
	
	register int x;
   while ((x = getopt (argc, argv, optString)) != EOF) {
	   
	   switch (x) {
	   case 'l':
		   globalArgs.langCode = optarg;
		   break;

	   case 'o':
		   globalArgs.outFileName = optarg;
		   break;

	   case 'v':
		   globalArgs.verbosity++;
		   break;
	case 'a':
		break;
	case 'h':
	case '?':
		helpme();			/* exits by itself */
	default:
		errno = 0;
		// bail ("nc -h for help");
		helpme();
    } /* switch x */
  } /* while getopt */
   globalArgs.inputFiles = argv + optind;
   globalArgs.numInputFiles = argc - optind;

	return 0;
}



