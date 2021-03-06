/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Gemeinsam verwendete Module
 * 
 * rfc.h: Definitionen für das Netzwerkprotokoll gemäß dem RFC
 */

#ifndef RFC_H
#define RFC_H
#include "common/question.h"


// maximale Anzahl an Spielern
#define MAX_PLAYERS	4

// max. Laenge Spielername (inkl. '\0')
#define PLAYER_NAME_LENGTH 32

// Padding durch Compiler verhindern
#pragma pack(push, 1)

#define RFC_BASE_SIZE 3
#define RFC_VERSION 7


// Uebersicht über die Nachrichtentypen
#define RFC_LOGINREQUEST         			1 // Anmeldung eines Clients am Server
#define RFC_LOGINRESPONSEOK			2 // Anmeldung am Server erfolgreich
#define RFC_PLAYERLIST							6 // Liste der Spielteilnehmer, wird versendet bei: An-/Abmeldung, Spielstart und Aenderung des Punktestandes



// LRQ LoginRequest Struktur
// Type = 1
// Length = Länge des Namens + 1
// RFCVersion = 7
// Login-Name, UTF-8, nicht nullterminiert, maximal 31 Bytes
#define RFC_LRQ_SIZE (RFC_BASE_SIZE + 1)
struct rfcLoginRequest {
	uint8_t rfcVersion;
	char loginName[31];
};

// LOK LoginResponseOk Struktur
// Type = 2
// Length = 2
// RFCVersion = 7
// ClientID = 0 (0 := Spielleiter)
#define RFC_LOK_SIZE (RFC_BASE_SIZE + 2)
struct rfcLoginResponseOk {
	uint8_t rfcVersion;
	uint8_t clientID;
};

//Anfang der Protokolle
typedef union {
	struct rfcLoginRequest lrq;
	struct rfcLoginResponseOk lok;
}CONTENT;


// Header der pakete
typedef struct rfcHeader {
	uint8_t type;
	uint16_t length;
}HEADER;


//komplette Struktur der Protokolle
typedef struct{
	HEADER header;
	CONTENT content;
}PACKET;





// Ende Pragma Block
#pragma pack(pop)


/* ******* Block fuer Funktions-Prototypen ******* */
/* *********************************************** */

// KontrollFunktion um Paketempfang zu ueberpruefen
int receiveMessage(int socket, PACKET *packet);

// KontrollFunktion fuer das Type-Feld des jeweiligen Datenpakets
int typeControl(struct rfcHeader header, uint8_t type);

#endif
