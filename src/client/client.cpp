#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"

int main() {
    // NSS initialisieren
    NSS_Init("");

    // SSL-Client erstellen
    PRFileDesc* clientSocket = SSL_ImportFD(nullptr, PR_NewTCPSocket());
    if (!clientSocket) {
        fprintf(stderr, "Fehler beim Erstellen des Client-Sockets\n");
        return 1;
    }

    // Mit dem Server verbinden
    PRNetAddr serverAddr;
    PR_InitializeNetAddr(PR_IpAddrAny, 0, &serverAddr);
    serverAddr.inet.family = PR_AF_INET;
    serverAddr.inet.port = PR_htons(443); // Setze den richtigen Port ein

    PRStatus status = PR_Connect(clientSocket, &serverAddr, PR_INTERVAL_NO_TIMEOUT);
    if (status != PR_SUCCESS) {
        fprintf(stderr, "Fehler beim Verbinden mit dem Server\n");
        return 1;
    }

    // Daten über die TLS-Verbindung senden/empfangen
    // Implementiere hier deine Logik für die Datenübertragung

    // Aufräumen
    PR_Close(clientSocket);
    NSS_Shutdown();

    return 0;
}
