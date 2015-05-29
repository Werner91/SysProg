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


// Basis-Struktur der RFC
struct rfcBase {
	uint8_t type;
	uint16_t length;
};

// LRQ LoginRequest Struktur
// Type = 1
// Length = Länge des Namens + 1
// RFCVersion = 7
// Login-Name, UTF-8, nicht nullterminiert, maximal 31 Bytes
#define RFC_LRQ_SIZE (RFC_BASE_SIZE + 1)
struct rfcLoginRequest {
	struct rfcBase base;
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
	struct rfcBase base;
	uint8_t rfcVersion;
	uint8_t clientID;
};

typedef union {
	struct rfcBase base;
	struct rfcLoginRequest lrq;
	struct rfcLoginResponseOk lok;
} rfc;

// Ende Pragma Block
#pragma pack(pop)


/* ******* Block fuer Funktions-Prototypen ******* */
/* *********************************************** */

// KontrollFunktion um Paketempfang zu ueberpruefen
int receiveMessage(int socket, rfc *packet);

// KontrollFunktion fuer das Type-Feld des jeweiligen Datenpakets
int typeControl(struct rfcBase base, uint8_t type);

#endif
