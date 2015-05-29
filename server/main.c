/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Server
 * 
 * main.c: Hauptprogramm des Servers
 */

#include "common/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>

#include "common/rfc.h"

void error(char *msg) { //wird aufgerufen, wenn ein Systemcall fehlschlägt
	perror(msg);
	exit(1);
}

void show_help() {
	printf("Usage: getopt [OPTIONS] [EXTRA]...\n");
	printf("       -p --port     Specify a portnumber (argument)\n");
	printf("\n");
	printf("       -h --help     show this help message\n");
	printf("\n");
	printf("Examples:\n");
	printf("       ./server --help\n");
	printf("       ./server --h\n");
	printf("       ./server --port 54321\n");
	printf("       ./server -p 54321\n");
}

/**************Singleton Pattern************************/
//#define LOCKFILE "/tmp/serverGroup04"
#define LOCKFILE "serverGroup04"

static int singleton(const char *lockfile) {

	int file = open(lockfile, O_WRONLY | O_CREAT, 0644);
	if (file < 0) {
		perror("PID-Datei konnte nicht erstellt werden");
		return 1;
	}

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	//Vorbereeiteter lock setzten
	if (fcntl(file, F_SETLK, &lock) < 0) {
		perror("PID Datei konnte nicht gelockt werden");
		return 2;
	}

	//Prüfen ob Datei leer ist
	if (ftruncate(file, 0) < 0) {
		perror("ftruncate");
		return 3;
	}

	//Schreibe die Prozess ID (PID) des Servers in die Datei
	char s[32];
	snprintf(s, sizeof(s), "%d\n", (int) getpid());

	if (write(file, s, strlen(s)) < strlen(s)) {
		perror("write");
	}

	if (fsync(file) < 0) {
		perror("fsync");
	}

	return 0;
}

/**************Singleton Pattern Ende************************/

int main(int argc, char **argv) {

	const char* short_options = "h:p:";
	int long_index = 0;
	int c;
	int sockfd, newsockfd, portno, n;
	socklen_t cliLen;
	char buffer[256]; //gelesene Character werden hier gespeichert (also die Nachricht)
	struct sockaddr_in serverAddr, clientAddr; //Struktur enthält die Internet-Adresse des Servers und des Clients

	const char *port = "54321";

	// Variable um Anzahl angemeldeter Spieler zu tracken
	int playerCount = 0;

	//PID Datei erzeugen
	if (singleton(LOCKFILE) != 0) {
		return 1;
	}

	/**************Verarbeitung Parameter aus der Konsole******************/
	struct option long_options[] = { //hier sind die langen Optionen wie z.b. --help gespeichert
			{ "help", optional_argument, 0, 'h' }, { "port", required_argument,
					0, 'p' }, // 1 = ein Argument
					{ NULL, 0, NULL, 0 } };

	if (argc <= 1) {
		printf("Keine Parameter wurden angegeben. Probiere: %s --help\n\n",
				argv[0]);
	}
	printf("(argc = %d)\n", argc);
	printf("Gefundene Parameter:\n");

	while (1) {
		c = getopt_long(argc, argv, short_options, long_options, &long_index); //getopt_long parst Parameter
		if (c == -1) { //Ende der Argumentenliste
			break;
		}

		switch (c) {
		case 'h': //hier sind die kurzen Optionen wie z.b. -h
			printf("  --help\n\n");
			show_help();
			exit(1);
			break;

		case 'p':
			if (optarg) {
				printf("  --port: %s\n", optarg);
				port = optarg;
			}
			break;

		case '?':
			//show_help();
			exit(1);
			//break;

		default:
			printf("Option derzeit nicht implementiert --%s (%c)\n",
					argv[optind - 1], c);

		}
	}

	/**************Konsole Parameter ende******************/

	setProgName(argv[0]);
	/* debugEnable() */

	infoPrint("Server Gruppe 4 (Stroh, Steinbinder, Strohm)");

	/*********************Verbindungsaufbau*************************************/

	/*Neuen Socket erstellen*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("FEHLER beim öffnen des Sockets");
	}

	/*Buffer löschen*/
	bzero((char *) &serverAddr, sizeof(serverAddr)); //Server Adresse auf 0.0.0.0 setzen

	portno = atoi(port); //String von der Konsole in Integer umwandeln

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portno); //Konvertiert Portnummer von host byte order in network byte order
	serverAddr.sin_addr.s_addr = INADDR_ANY; //IP-Adresse auf dem der Server läuft wird eingefügt

	if (bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		error("FEHLER beim binding");
	}

	listen(sockfd, 5);

	cliLen = sizeof(clientAddr);
	newsockfd = accept(sockfd, NULL, NULL);
	if (newsockfd < 0) {
		error("FEHLER bei accept");
	}

	// Warten auf LoginRequests
	rfc lrq;
	printf("Vor dem receive\n");
	int receive = receiveMessage(newsockfd, &lrq);
	printf("Nach dem receive I\n");
	if (receive == -1) {
		error("ERROR: Fehler beim Empfang eines LoginRequests server/main.c");
		return 1;
	} else if (receive == 0) {
		error("ERROR: Verbindung fehlgeschlagen LoginRequest  server/main.c");
		return 1;
	}
	printf("Laenge: %d\n", lrq.base.length);
	printf("Typ: %d\n", lrq.base.type);

	printf("Nach dem receive II\n");

	if (typeControl(lrq.base, 1)) {
		if (lrq.lrq.rfcVersion != RFC_VERSION) {
			error("ERROR: Falsche RFC Version");
			return (1);
		}
		printf("Versionskontrolle hat funktioniert\n");

		int length = ntohs(lrq.base.length) - 1;
		char s[length + 1];
		s[length] = '\0';
		memcpy(s, lrq.lrq.loginName, length);
		printf("memcpy hat funktioniert\n");

		printf("Spielername lautet %s\n", s);

		struct rfcLoginResponseOk lok;
		lok.base.type = 2;
		lok.base.length = 2;
		lok.clientID = 1;
		lok.rfcVersion = 7;

		printf("Vor dem zweiten Send\n");
		if (send(sockfd, &lok, RFC_LOK_SIZE, 0) == -1) {
		            errnoPrint("send");
		            return 1;
		        }
		printf("Nach dem zweiten Send\n");
	}

	bzero(buffer, 256);

	n = read(newsockfd, buffer, 255);
	if (n < 0) {
		error("FEHLER beim lesen vom socket");
	}

	printf("\n\n");
	printf("Nachricht: %s\n\n\n", buffer);

	n = write(newsockfd, "Ich hab deine Nachricht erhalten!", 33);
	if (n < 0) {
		error("FEHLER beim schreiben auf den socket");
	}

	return 0;
}
