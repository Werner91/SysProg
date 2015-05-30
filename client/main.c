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
	printf("Ohne Paramter -p wird der Standartport 54321 verwendet\n\n\n");

}

/*****************************Verbindung herstellen ****************************************/
int verbindung_herstellen(int _socketdeskriptor, char* _port, char* _ipadresse){

	struct sockaddr_in serv_addr; // IP-Adress-Struktur
	struct hostent *server;
	int portno;
	int fehler = 0;

	portno = atoi(_port);
	server = gethostbyname(_ipadresse);

	if(server == NULL){
		infoPrint("Host konnte nicht gefunden werden");
		fehler = 1;
	}else{

	// Server Adresse setzen
		bzero((char *) &serv_addr, sizeof(serv_addr));
		// IPv4
		serv_addr.sin_family = AF_INET;
		bcopy(server->h_addr, &serv_addr.sin_addr, server->h_length);

		// Port setzen
		portno = atoi(_port);
		serv_addr.sin_port = htons(portno);

		// Verbindung pruefen
		printf("Verbindung wird aufgebaut...\n");

		//Ausagbe zur Fehlersuche
		printf("Socket: %d\n", _socketdeskriptor);
			printf("port: %s\n",_port );
			printf("ipadresse: %s\n", _ipadresse);


		if (connect(_socketdeskriptor, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
			error("ERROR: Fehler beim Verbindungsaufbau client/main.c\n");
			fehler = 1;
		}
	}
	return fehler;
}

/***************************** login request senden****************************************/

void sende_login_request(char* _name, int _socketDeskriptor){

	// Uebertragung LoginRequest
	// Paket vorbereiten
	PACKET lrq;
	int nameLength = strlen(_name);
	lrq.header.length = htons(nameLength + 1);
	lrq.header.type = RFC_LOGINREQUEST;
	memcpy(lrq.content.lrq.loginName, _name, nameLength);
	lrq.content.lrq.rfcVersion = RFC_VERSION;
	printf("Laenge des LRQ: %d\n", lrq.header.length);
	printf("Login gesendet\n");

	// Eigentliche Uebertragung
	if (send(_socketDeskriptor, &lrq, ntohs( RFC_LRQ_SIZE) + strlen(_name), 0) == -1) {
		infoPrint("ERROR: Fehler bei beim uebertragen der LRQ Nachrichten an den Server\n");
		exit (0);
	}else{
		infoPrint("LRQ Nachricht erfolgreich gesendet\n");
	}

}





int main(int argc, char **argv) {
	setProgName(argv[0]);
	debugEnable();

	guiInit(&argc, &argv);
	infoPrint("Client Gruppe 4 (Stroh, Steinbinder, Strohm)\n");

	int sockfd; // Anlegen SocketFileDeskriptor, Portnummer
	int c; //Parameter für getopt
	int long_index = 0; //Parameter für getopt

	char *name = "unknown";
	char *ipadresse = "localhost";
	char *port = "54321";
	int port_ueberpruefung; // Um den eingegebenen Port auf richtigkeit zu prüfen
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
		case 'n':
			printf("LÄUFT --name\n\n");
			name = optarg;
			//strncpy(username, optarg, 31);
			if((strlen(name))>=31){
				infoPrint("Der gewaehlte Name ist zu lang! Max. 31 Zeichen erlaubt\n");
				exit(1);
			 }
			userNameIsSet = 1;
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

	// Ueberpruefen ob ein Username gesetzt wurde ansonsten Hilfe ausgeben
	if (userNameIsSet == 0) {
		printf("\nEs wurde kein Name angegeben\n\n");
		show_help();
		exit(0);
	}

	// Socket anlegen, Rueckgabe ist der Filedeskriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		error("ERROR: Socket-Erzeugung client/main.c\n");
		exit(0);
	}


	if(verbindung_herstellen(sockfd, port, ipadresse) == 0) {

	printf("Verbindung wurde aufgebaut\n");

	// GUI initialisieren
	infoPrint("initialisiere GUI\n");
	//guiInit(&argc, &argv);

	//Erstellen des Login Request
	infoPrint("sende LoginRequest");
	sende_login_request(name, sockfd);

 	preparation_setMode(PREPARATION_MODE_BUSY);
	preparation_showWindow();




	// BIS HIERHER LAEUFTS



	/*
	// Antwort vom Server
	listen(sockfd, 128);
	int newsockfd = accept(sockfd, NULL, NULL);
	if (newsockfd < 0) {
		error("FEHLER bei accept");
	}
	 */



	PACKET lok;
	lok.header.type = htons(2);
	int receive = receiveMessage(sockfd, &lok);

	if (receive == -1) {
		printf("Verbindung verloren client/main.c\n");
		return 1;
	} else if (receive == 0) {
		errorPrint("Uebertragung fehlgeschlagen \n");
		return 1;
	}

	// RFCVersionskontrolle
	//if (typeControl(lok.base, 2))
	if (typeControl(lok.header, 2)) {
		if (lok.content.lok.rfcVersion != RFC_VERSION) {
			error("ERROR: Falsche RFC Version");
			return (1);
		}

		// Speichere zugewiesene clientID
		clientID = lok.content.lok.clientID;
		infoPrint("Ihre ClientID: %d", clientID);
		printf("Vor dem AddPlayer der GUI");
		preparation_addPlayer(name); // Spielername in der GUI anzeigen
		if (lok.content.lok.clientID == 0) {
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
	}
	close(sockfd);
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
