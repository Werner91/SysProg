/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Client
 * 
 * main.c: Hauptprogramm des Clients
 */

#include "common/util.h"
#include "gui/gui_interface.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(char *msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char **argv) {
	setProgName(argv[0]);
	debugEnable();

	guiInit(&argc, &argv);
	infoPrint("Client Gruppe 4 (Stroh, Steinbinder, Strohm)");

	int sockfd, portno, n; // Anlegen SocketFileDeskriptor, Portnummer
	struct sockaddr_in serv_addr; // IP-Adress-Struktur
	struct hostent *server; // Struktur fuer
	char buffer[256]; // Pufferspeicher fuer eingelesene Nachricht

	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]); // Portnummer
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Socket anlegen, Rueckgabe ist der Filedeskriptor

	if (sockfd < 0) {
		error("ERROR: Socket-Erzeugung client/main.c\n");
	}

	server = gethostbyname(argv[1]); // argv[1] Name des Hosts
	if (server == NULL) {
		fprintf(stderr, "ERROR: Host nicht verfuegbar client/main.c\n");
		exit(0);
	}

	// Server Adresse setzen
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; // IP Version 4
	bcopy((char *) server->h_addr,
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length); // Laenge der IP-Adresse
	serv_addr.sin_port = htons(portno); // Port setzen

	// Verbindung pruefen
	if (connect(sockfd, &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR: Fehler beim Verbindungsaufbau client/main.c\n");
	}

	// Eigentlich zu uebertragende Nachricht
	printf("Please enter the message: ");
	bzero(buffer, 256);
	fgets(buffer, 255, stdin);
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0){
		error("ERROR: Fehler bei Nachrichtenuebertragung client/main.c\n");
	}
	bzero(buffer, 256);
	n = read(sockfd, buffer, 255);
	if (n < 0){
		error("ERROR: Fehler beim Lesevorgang client/main.c\n");
	}
	printf("%s\n", buffer);

	/* Initialisierung: Verbindungsaufbau, Threads starten usw... */

	preparation_showWindow();
	guiMain();

	/* Resourcen freigeben usw... */
	guiDestroy();

	return 0;
}

/* Vordefinierte Funktionen fuer entsprechende Events */

void preparation_onCatalogChanged(const char *newSelection) {
	debugPrint("Katalogauswahl: %s", newSelection);
}

void preparation_onStartClicked(const char *currentSelection) {
	debugPrint("Starte Katalog %s", currentSelection);
}

void preparation_onWindowClosed(void) {
	debugPrint("Vorbereitungsfenster geschlossen");
	guiQuit();
}

void game_onSubmitClicked(unsigned char selectedAnswers) {
	debugPrint("Absende-Button angeklickt, Bitmaske der Antworten: %u",
			(unsigned) selectedAnswers);
}

void game_onWindowClosed(void) {
	debugPrint("Spielfenster geschlossen");
	guiQuit();
}
