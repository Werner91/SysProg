/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Server
 * 
 * login.c: Implementierung des Logins
 */

#include "login.h"
#include  <stdbool.h>
#include "common/rfc.h"
#include  "user.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>


// Spielstatus verwalten
bool game_running = false;


/*
 * Login_Thread Start
 *
 * param Serversocket
 */
void* login_main(int server_socket){

	int client_socket;
	rfc lrq;
	int receive;
	int client_id;
	rfc response;
	rfc lok;

	// Threadverwaltung - je Client ein Thread - Thread verwaltet Spielphase
		pthread_t client_threads[MAX_PLAYERS];

	// ------------------------------> Initialisiere Userdaten-Mutex <------------------------------
			if(create_user_mutex() == -1){
				// sollte initialisieren nicht erfolgreich sein terminiere Thread
				error("Thread konnte nicht erstellt werden -> terminiert");
				pthread_exit(NULL);
			}

			// Empfaenger-Schleife
				while(1){
					// Warte auf Connection-Request
					printf("Server wartet auf Verbindungsanfragen...\n");
					if(listen(server_socket,64) == 0){
						// Bei erfolgreichem Request wird Connection aufgebaut
						client_socket = accept(server_socket, 0, 0);
						if(client_socket == -1){
							errorPrint("Client Socket Connection Fehler\n");
							exit(0);
						}else{
							infoPrint("Server ist jetzt mit Client Verbunden\n");
						}
					// Verbidung hergestellt
					infoPrint("Verbindung hergestellt, warte auf Loginrequst...\n");

					// Empfange Paket
					receive = receiveMessage(client_socket, &lrq);
					if (receive == -1) {
						error("ERROR: Fehler beim Empfang eines LoginRequests server/main.c");
						exit(0);
					} else if (receive == 0) {
						error("ERROR: Verbindung fehlgeschlagen LoginRequest  server/main.c");
						exit(0);
					}else{
						infoPrint("LoginRequest ist angekommen\n");
					}


					if(game_running == false){
						// fuege Spieler zur Verwaltung hinzu, uebergebe Name + Socketdeskriptor
						lock_user_mutex();
						client_id = addPlayer(lrq.lrq.loginName, ntohs(lrq.lrq.base.length), client_socket);
						unlock_user_mutex();

						//Name bereits vorhanden
						if(client_id == -1){
							errorPrint("Falscher Name oder Name bereits vergeben");
							//ERROR NACHRICHTEN FEHLEN HIER NOCH
							exit(0);
						//Zuviele Spieler angemeldet
						}else if(client_id >= MAX_PLAYERS){
							errorPrint("Maximale Anzahl an Spieler erreicht!");
							//ERROR NACHRICHTEN FEHLEN HIER NOCH
							exit(0);
						//ID ok
						}else{
							infoPrint("Spieler erfolgreich hinzugefuegt - Client_ID: %i", client_id);
							response.lok.base.type = RFC_LOGINRESPONSEOK;
							response.lok.base.length = htons(2);
							response.lok.clientID = client_id;
							response.lok.rfcVersion = RFC_VERSION;
						}

						//sofern Anmeldung ok - erstelle Clientthread fuer neu hinzugefuegten Spieler
						if((client_id >= 0) && (client_id < MAX_PLAYERS )){
							//uebergebe Client-ID an Client-Thread
							infoPrint("Erstelle Client-Thread");
					/*
							pthread_create(&client_threads[client_id], NULL, (void*) &client_thread_main, &client_id);
						}else{
							error("Client-Thread erstellen fehlgeschlafen, vielleicht ein fehler in der if() abfrage?");
					*/
						}

						//sende Antwort
							// sende Daten ueber Socket
							if(send(client_socket, &response, ntohs(response.base.length)+3,0) == -1){
								errorPrint("Senden der Daten fehlgeschlagen!");
								exit(0);
							}
							else {
								// Testweise ausgeben welcher Typ an welchen Socket versendet wurde
								infoPrint("Nachicht vom Type %d an die Socket-ID: %i gesendet ", response.lok.base.type, client_socket);
							}

						}

					}
				}
			//terminiere Thread
			pthread_exit(NULL);
			return NULL;
}
