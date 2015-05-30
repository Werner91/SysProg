/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 * Achim Strohm, Werner Steinbinder, Michael Stroh
 *
 * Gemeinsam verwendete Module
 * 
 * rfc.c: Implementierung der Funktionen zum Senden und Empfangen von
 * Datenpaketen gemäß dem RFC
 */

#include <sys/types.h>
#include <sys/socket.h>
#include "rfc.h"

// KontrollFunktion um Paketempfang zu ueberpruefen
// Rueckgabe -1 bei Fehler Paketempfang
// Rueckgabe  0 falls kein Paket verfuegbar bzw Verbindung unterbrochen
// Rueckgabe  1 falls Paketuebertragung erfolgreich war
int receiveMessage(int socket, PACKET *packet) {
	void* packetbuffer = packet;
	// recv - receive a message from a connected socket
	// recv(int socket, void *buffer, size_t length, int flags);
	int receivePacket = recv(socket, packetbuffer, RFC_BASE_SIZE, 0);
	if (receivePacket == -1) {
		error("rfc.c/receiveMessage: Fehler beim Paketempfang I"); // Laut Anleitung errno
		return -1;
	}
	if (receivePacket == 0) {
		// Keine Pakete verfuegbar oder Verbindung unterbrochen
		return 0;
	}

	int packetLength = ntohs(packet->header.length);

	if (packetLength > 0) {
		receivePacket = recv(socket, packetbuffer + RFC_BASE_SIZE, packetLength, 0);
		if (receivePacket == -1) {
			error("rfc.c/receiveMessage: Fehler beim Paketempfang II"); // Laut Anleitung errno
			return -1;
		}
		if (receivePacket == 0) {
			// Keine Pakete verfuegbar oder Verbindung unterbrochen
			return 0;
		}
	}
	return 1;
}


// KontrollFunktion fuer das Type-Feld des jeweiligen Datenpakets
// Rueckgabe 1 bei Uebereinstimmung des Type-Felds
// Rueckgabe 0 bei falschem Type-Feld
int typeControl(struct rfcHeader header, uint8_t type) {
	printf("rfcHeader.type: %d\n", header.type);
	printf("type: %d\n", type);
	if (header.type == type) {
		printf("Typfeld stimmt ueberein\n");
		return 1;
	} else {
		printf("Typfeld stimmt nicht ueberein\n");
		return 0;
	}
}
