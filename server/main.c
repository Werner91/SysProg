/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * main.c: Hauptprogramm des Servers
 */

#include "common/util.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket>
#include <netinet/in.h>



void error(char *msg){ //wird aufgerufen, wenn ein Systemcall fehlschlägt
	perror(msg);
	exit(1);
}


int main(int argc, char **argv)
{
	setProgName(argv[0]);
	/* debugEnable() */

	infoPrint("Server Gruppe 4 (Stroh, Steinbinder, Strohm)");


	int sockfd, newsockfd, portno, clillen, n;
	char buffer[256]; //gelesene Character werden hier gespeichert (also die Nachricht)

	struct sockaddr_in serverAddr, clientAddr; //Struktur enthält die Internet-Adresse des Servers und des Clients

	if(argc < 2){
		fprintf(stderr, "Fehler, keine Portnummer eingegeben\n");
		exit(1);
	}



	return 0;
}
