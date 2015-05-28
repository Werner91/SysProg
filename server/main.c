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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>

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
	printf("       ./server --port 8111\n");
	printf("       ./server -p 8111\n");
}

int main(int argc, char **argv) {

	const char* short_options = "h:p:";
	int long_index = 0;
	int c;
	int sockfd, newsockfd, portno, cliLen, n;
	char buffer[256]; //gelesene Character werden hier gespeichert (also die Nachricht)
	struct sockaddr_in serverAddr, clientAddr; //Struktur enthält die Internet-Adresse des Servers und des Clients

	char *standardPort = "8111";

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

	portno = atoi(standardPort); //String von der Konsole in Integer umwandeln

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portno); //Konvertiert Portnummer von host byte order in network byte order
	serverAddr.sin_addr.s_addr = INADDR_ANY; //IP-Adresse auf dem der Server läuft wird eingefügt

	if (bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		error("FEHLER beim binding");
	}

	listen(sockfd, 5);

	cliLen = sizeof(clientAddr);
	newsockfd = accept(sockfd, (struct sockaddr *) &clientAddr, &cliLen);
	if (newsockfd < 0) {
		error("FEHLER bei accept");
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
