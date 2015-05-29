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
#include "common/util.h"

#include "common/rfc.h"
#include "user.h"

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
	//socklen_t cliLen;
	struct sockaddr_in serverAddr; //clientAddr; //Struktur enthält die Internet-Adresse des Servers und des Clients
	int port_ueberpruefung; // Den eingegebenen Port auf richtigkeit prüfen
	const char *port = "54321";

	// Variable um Anzahl angemeldeter Spieler zu tracken
	// Bisher nicht in Benutzung
	// int playerCount = 0;

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
			if (isOnlyNumber(optarg)) {
							printf("LÄUFT --port\n\n");
							port = optarg;
						} else {
							errorPrint("\nPortnummer darf nur aus Zahlen bestehen!\n");
							exit(1);
						}
						port_ueberpruefung = atoi(optarg);
						if((port_ueberpruefung < 65535) && (port_ueberpruefung > 4000)){
							port = optarg;
						}
						else {
							infoPrint("Port muss zwischen 4000 - 65535 sein!");
							infoPrint("Es wird der Standardport 54321 verwendet");
							port = "54321";
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


	// Spielerverwaltung initialisieren
		initSpielerverwaltung();

	/*********************Verbindungsaufbau*************************************/

	/*Neuen Socket erstellen*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("FEHLER beim öffnen des Sockets");
		// SERVER BEENDEN FEHLT NOCH AN DIESER STELLE beendeServer()
		exit(0);
	}

	// Struct fuer Socketadresse
		struct sockaddr_in address;

		// Addresseigenschaften
			// Erstellt Speicherplatz für Adresse und fuellt mit Nullen
			memset(&address, 0, sizeof(address));
			// IPv4 Adresse
			address.sin_family = AF_INET;
			// // konvertiere Werte von host byte order zu network byte order
			int server_port = atoi(port);
			address.sin_port = htons(server_port);
			// Empfaengt von allen Interfaces
			address.sin_addr.s_addr = INADDR_ANY;

			// Socket an den Port binden
			if (bind(sockfd, (struct sockaddr *) &address, sizeof(address)) == -1) {
				errorPrint("bind error: Möglicherweise ist dieser Port (noch) in verwendung\n");
				close(sockfd);
				return -1;
			}
			// testweise Serverport ausgeben
			infoPrint("Serverport: %s", port);


	// starte Login-Thread
	pthread_t login_thread;
	printf("Versuche Login Thread zu erstelt...\n");
	if(pthread_create(&login_thread, NULL, (void*)login_main(sockfd), NULL) == -1){
			errorPrint("Login_thread konnte nicht erstellt werden!");
			// SERVER BEENDEN FEHLT NOCH AN DIESER STELLE beendeServer()
			exit(0);
	}else{
		printf("Login Thread wurde erstelt\n");
	}








		/*
		printf("Vor dem zweiten Send\n");
		if (send(sockfd, &lok, RFC_LOK_SIZE, 0) == -1) {
		            errnoPrint("send");
		            return 1;
		        }
		printf("Nach dem zweiten Send\n");


	}

		 */


/*
	n = write(newsockfd, "Ich hab deine Nachricht erhalten!", 33);
	if (n < 0) {
		error("FEHLER beim schreiben auf den socket");
	}
*/
	return 0;
}
