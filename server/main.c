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



void error(char *msg){ //wird aufgerufen, wenn ein Systemcall fehlschlägt
	perror(msg);
	exit(1);
}


int main(int argc, char **argv)
{
	setProgName(argv[0]);
	/* debugEnable() */

	infoPrint("Server Gruppe 4 (Stroh, Steinbinder, Strohm)");


	int sockfd, newsockfd, portno, cliLen, n;
	char buffer[256]; //gelesene Character werden hier gespeichert (also die Nachricht)

	struct sockaddr_in serverAddr, clientAddr; //Struktur enthält die Internet-Adresse des Servers und des Clients

	if(argc < 2){
		fprintf(stderr, "FEHLER, keine Portnummer eingegeben\n");
		exit(1);
	}

	/*Neuen Socket erstellen*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		error("FEHLER beim öffnen des Sockets");
	}

	/*Buffer löschen*/
	bzero((char *) &serverAddr, sizeof(serverAddr)); //Server Adresse auf 0.0.0.0 setzen

	portno = atoi(argv[1]); //String von der Konsole in Integer umwandeln


	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portno); //Konvertiert Portnummer von host byte order in network byte order
	serverAddr.sin_addr.s_addr = INADDR_ANY; //IP-Adresse auf dem der Server läuft wird eingefügt


	if(bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) <0){
		error("Fehler");
	}

	listen(sockfd, 5);

	cliLen = sizeof(clientAddr);
	newsockfd = accept(sockfd, (struct sockaddr *) &clientAddr, &cliLen);
	if (newsockfd < 0){
		error("FEHLER bei accept");
	}

	bzero(buffer,256);

	n = read(newsockfd, buffer, 255);
	if(n < 0){
		error("FEHLER beim lesen vom socket");
	}

	printf("\n\n");
	printf("Nachricht: %s\n\n\n", buffer);


	n = write(newsockfd, "Ich hab deine Nachricht erhalten!",33);
	if(n < 0){
		error("FEHLER beim schreiben auf den socket");
	}

	return 0;
}
