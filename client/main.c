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
#include <getopt.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "common/rfc.h"

void error(char *msg) {
	perror(msg);
	exit(0);
}

void show_help() {
	printf("Bitte geben sie Folgende Startparameter an:\n");
	printf("Angaben in [] sind optional\n\n");
	printf("	   -n  --name 		--> Spielername\n");
	printf("	  [-p] --port		--> Welcher Port verwendet werden soll\n");
	printf("	  [-h] --hilfe 		--> zeigt diese Hilfemeldung an\n");
	printf("	  [-i] --ipadresse	--> Welche IP Adresse verwendet werden soll");
	printf("\nBeispiel: ./Client -n Spielername -p 54321\n");
	printf("Beispiel: ./Client --name Spieldername --port 54321\n");
	printf("Ohne Paramter -p wird der Standartport 8111 verwendet\n\n\n");
}

int main(int argc, char **argv) {
	setProgName(argv[0]);
	debugEnable();

	guiInit(&argc, &argv);
	infoPrint("Client Gruppe 4 (Stroh, Steinbinder, Strohm)");

	int sockfd; // Anlegen SocketFileDeskriptor, Portnummer
	struct sockaddr_in serv_addr; // IP-Adress-Struktur
	struct hostent *server;
	int c; //Parameter für getopt
	int long_index = 0; //Parameter für getopt

	char *name = "unknown";
	char username[31];
	username[31] = '\0';
	char *ipadresse = "localhost";
	char *port = "54321";
	int userNameIsSet = 0;
	int clientID;

	/*
	 * Wird verwendet um kurze (getopt) Paramter (-n) oder auch lange Paramter (--name) auszuwerten
	 */
	const char* short_options = "h:p:n:i:";
	static struct option long_options[] = { { "hilfe", no_argument, 0, 'h' }, {
			"port", optional_argument, 0, 'p' }, { "name", required_argument, 0,
			'n' }, { "ipadresse", optional_argument, 0, 'i' }, { 0, 0, 0, 0 } };

	if (argc <= 1) {
		printf("Es wurden keine Paramter angegeben.\n");
		show_help();
		exit(0);
	}
	printf("Test --> Anzahl der Paramter:%d \n\n", argc);
	printf("Test --> Gefundene Paramter:\n");

	while (1) {

		//Argumente auslesen mit hilfe von getopt.h
		c = getopt_long(argc, argv, short_options, long_options, &long_index);
		if (c == -1) { //Ende der Argumentzeile erreicht?
			break;
		}

		switch (c) {
		case 'h':
			printf("LÄUFT --help\n\n");
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
			break;
		case 'n':
			printf("LÄUFT --name\n\n");
			userNameIsSet = 1;
			strncpy(username, optarg, 31);
			/*if((strlen(name))>31){
			 printf("Der gewaehlte Name ist zu lang! Max. 31 Zeichen erlaubt\n");
			 exit(0);
			 }*/
			break;
		case 'i':
			printf("LÄUFT --ipadresse\n\n");
			ipadresse = optarg;
			break;
		default:
			printf("\nUngueltiger Parameter\n\n");
			break;
		}

	}

	printf("name:%s\n", name);
	printf("IP:%s\n", ipadresse);
	printf("Port:%s\n", port);

	//Ueberprüfen ob ein Username gesetzt wurde ansonsten Hilfe ausgeben
	if (userNameIsSet == 0) {
		printf("Es wurde kein Name angegeben");
		show_help();
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Socket anlegen, Rueckgabe ist der Filedeskriptor

	if (sockfd < 0) {
		error("ERROR: Socket-Erzeugung client/main.c\n");
	}

	// Server Adresse setzen
	bzero((char *) &serv_addr, sizeof(serv_addr));
	server = gethostbyname(ipadresse);
	//bcopy(server->h_addr, &serv_addr.sin_addr, server->h_length);
	bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_family = AF_INET; // IP Version 4
	//serv_addr.sin_addr.s_addr = *ipadresse;
	uint16_t final_port = atoi(port);
	serv_addr.sin_port = htons(final_port); // Port setzen

	// Verbindung pruefen
	printf("Verbindung wird aufgebaut...\n");
	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR: Fehler beim Verbindungsaufbau client/main.c\n");
	}
	printf("Verbindung wurde aufgebaut\n");

	// GUI initialisieren
	/*guiInit(&argc, &argv);
	 preparation_setMode(PREPARATION_MODE_BUSY);
	 preparation_showWindow();
	 */

	//Uebertragung LoginRequest
	struct rfcLoginRequest lrq;
	int nameLength = strlen(username);
	lrq.base.length = htons(nameLength + 1);
	lrq.base.type = 1;
	memcpy(lrq.loginName, username, nameLength);
	lrq.rfcVersion = RFC_VERSION;
	printf("Laenge des LRQ: %d\n", lrq.base.length);
	printf("Login gesendet\n");

	//An den Server senden
	if (send(sockfd, &lrq, RFC_LRQ_SIZE + strlen(username), 0) == -1) {
		error("ERROR: Fehler bei Nachrichten uebertragung client/main.c\n");
		return (1);
	}

	// BIS HIERHER LAEUFTS

	//Antwort vom Server
	listen(sockfd, 5);
	int newsockfd = accept(sockfd, NULL, NULL);
	if (newsockfd < 0) {
		error("FEHLER bei accept");
	}

	rfc lok;
	int receive = receiveMessage(newsockfd, &lok);

	if (receive == -1) {
		printf("Verbindung verloren client/main.c\n");
		return 1;
	} else if (receive == 0) {
		errorPrint("Uebertragung fehlgeschlagen \n");
		return 1;
	}

	//RFC Versions ueberprüfung
	if (typeControl(lok.base, 2)) {
		if (lok.lok.rfcVersion != RFC_VERSION) {
			error("ERROR: Falsche RFC Version");
			return (1);
		}

		// Speichere zugewiesene clientID
		clientID = lok.lok.clientID;
		infoPrint("Ihre ClientID: %d", clientID);
		printf("Vor dem AddPlayer der GUI");
		preparation_addPlayer(name); //Name wird der GUI hinzugefügt
		if (lok.lok.clientID == 0) {
			preparation_setMode(PREPARATION_MODE_PRIVILEGED);
		} else {
			preparation_setMode(PREPARATION_MODE_NORMAL);
		}
	}

	/* Initialisierung: Verbindungsaufbau, Threads starten usw... */

	//preparation_showWindow();
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
