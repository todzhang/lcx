#include "lcx.h"

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
	const char* bindAddr = args.listenHost;
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
	char* passwd = NULL;
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