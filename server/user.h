/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Server
 * 
 * user.h: Header für die User-Verwaltung
 */

#include "common/rfc.h"

#ifndef USER_H
#define USER_H

// Struct fuer Spielerverwaltung - Name, ID, Socket, Score, Flag fuer Spielende
typedef struct player {
	int id;
	char name[PLAYER_NAME_LENGTH];
	int sockDesc;
	uint32_t score;
	int GameOver;
} PLAYER;



void initSpielerverwaltung();
int create_user_mutex();
int addPlayer(char *name, int length, int sock);
void lock_user_mutex();
void unlock_user_mutex();




#endif
