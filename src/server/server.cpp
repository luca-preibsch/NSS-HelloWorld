#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"
#include "../../include/nspr.h"

int main() {
    // must be called before any other NSS function
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

//    char* db_dir = "./";
//    NSS_Init("./");

}

//int main() {
//    // NSS initialisieren
//    NSS_Init("");
//
//    // SSL-Server erstellen
//    PRFileDesc* serverSocket = SSL_CreateServerSocket(PR_AF_INET, PR_STREAM_LISTEN, 0, 443, nullptr);
//    if (!serverSocket) {
//        fprintf(stderr, "Fehler beim Erstellen des Server-Sockets\n");
//        return 1;
//    }
//
//    // Auf Verbindungen warten
//    PRFileDesc* sslSocket = SSL_ImportFD(nullptr, serverSocket);
//    PRStatus status = PR_Accept(sslSocket, nullptr, PR_INTERVAL_NO_TIMEOUT);
//    if (status != PR_SUCCESS) {
//        fprintf(stderr, "Fehler beim Akzeptieren der Verbindung\n");
//        return 1;
//    }
//
//    // Daten über die TLS-Verbindung senden/empfangen
//    // Implementiere hier deine Logik für die Datenübertragung
//
//    // Aufräumen
//    PR_Close(serverSocket);
//    NSS_Shutdown();
//
//    return 0;
//}
