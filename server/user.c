/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Server
 * 
 * user.c: Implementierung der User-Verwaltung
 */

#include "user.h"
#include "common/rfc.h"

#include "common/util.h"
#include "login.h"
#include "catalog.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

// Array fuer die Spielerverwaltung
PLAYER spieler[MAX_PLAYERS];

// Mutex fuer die Spielerverwaltung
pthread_mutex_t user_mutex;



void initSpielerverwaltung(){
	debugPrint("Initialisiere Spielervewaltung.");
		// initialisiere Spieler mit Standardwerten
		for(int i = 0; i < MAX_PLAYERS; i++){
			spieler[i].id = -1;
			spieler[i].name[0] = '\0';
			spieler[i].sockDesc = 0;
			spieler[i].score = 0;
			spieler[i].GameOver = 0;
		}
}


/*
 * Funktion initalisiert Mutex fuer die Benutzerdaten
 */
int create_user_mutex(){
	// initialisiere Mutex, NULL -> Standardwerte
	// http://www.lehman.cuny.edu/cgi-bin/man-cgi?pthread_mutex_init+3
	debugPrint("Initialisiere Benutzermutex.");
	if(pthread_mutex_init(&user_mutex, NULL) != 0){
		errorPrint("Fehler beim initialisieren des Benutzermutex.");
		return -1;
	}
	return 0;
}

/*
 * Funktion sperrt Mutex fuer die Benutzerdaten
 */
void lock_user_mutex(){
	debugPrint("lock Benutzerdatenmutex.");
	// lock mutex
	// http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html
	pthread_mutex_lock(&user_mutex);
}


/*
 * Funktion zaehlt aktuell angemeldete Spieler
 */
int countUser(){
	debugPrint("Zaehle aktuell angemeldete Spieler.");
	int current_user_count = 0;
	for(int i=0;i< MAX_PLAYERS;i++){
		// Spieler vorhanden - erhoehe Zaehler
		if((spieler[i].id != -1) && (spieler[i].sockDesc != 0)){
			current_user_count++;
		}
	}
	// gebe Anzahl an Spielern zurueck
	return current_user_count;
}

/*
 * Fuegt Spieler zur UserListe hinzu
 *
 * char* name Name des Spielers
 * int length Laenge des Namen
 * int sock Socket des Clients mit diesem Namen
 *
 * return ID des Users, falscher Name(-1) oder max. Anzahl an Spieler erreicht (4)
 */
int addPlayer(char *name, int length, int sock){
	debugPrint("Fuege Spieler zur Verwaltung hinzu.");
	name[length] = 0;
	int current_count_user = countUser();
	// sind noch freie Spielerplaetze vorhanden
	if(current_count_user >= MAX_PLAYERS){
		return MAX_PLAYERS;
	}
	else {
		// pruefe auf gleichen Namen
		for(int i = 0; i < current_count_user; i++){
			if(strncmp(spieler[i].name, name, length + 1) == 0){
				return -1;
			}
		}
		// fuege Spieler zur Verwaltung hinzu
		int new_id = current_count_user;
		spieler[new_id].id = new_id;
		strncpy(spieler[new_id].name, name, length + 1);
		spieler[new_id].sockDesc = sock;
		// gebe Spieler-ID zurueck
		return new_id;
	}
}

/*
 * Funktion gibt Mutex fuer die Benutzerdaten frei
 */
void unlock_user_mutex(){
	debugPrint("unlock Benutzerdatenmutex.");
	// unlock mutex
	// http://linux.die.net/man/3/pthread_mutex_unlock
	pthread_mutex_unlock(&user_mutex);
}
